// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "MinimumNodeService.h"
#include "Controller.h"
#include <cbusdefs.h>

namespace VLCB
{

void MinimumNodeService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

// TODO: This list is used while implementing MNS. Remove once done.
// MNS shall implement these opcodes in incoming requests
// * SNN - Done
// * QNN - Done
// * RQNP - Done
// * RQMN - Done
// * RQNPN - Done
// * NNRSM
// * RDGN
// * RQSD
// * MODE
// * SQU
// * NNRST
// * NNRSM
//
// CBUS - optional
// These shall be moved to CAN service.
// * CANID - Here
// * ENUM - Here

Processed MinimumNodeService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {

    case OPC_RQNP:
      // RQNP message - request for node parameters -- does not contain a NN or EN, so only respond if we
      // are in transition to FLiM
      // DEBUG_SERIAL << F("> RQNP -- request for node params during FLiM transition for NN = ") << nn << endl;

      // only respond if we are in transition to FLiM mode
      if (controller->bModeChanging)
      {
        // DEBUG_SERIAL << F("> responding to RQNP with PARAMS") << endl;

        // respond with PARAMS message
        msg->len = 8;
        msg->data[0] = OPC_PARAMS;    // opcode
        msg->data[1] = controller->_mparams[1];     // manf code -- MERG
        msg->data[2] = controller->_mparams[2];     // minor code ver
        msg->data[3] = controller->_mparams[3];     // module ident
        msg->data[4] = controller->_mparams[4];     // number of events
        msg->data[5] = controller->_mparams[5];     // events vars per event
        msg->data[6] = controller->_mparams[6];     // number of NVs
        msg->data[7] = controller->_mparams[7];     // major code ver
        // final param[8] = node flags is not sent here as the max message payload is 8 bytes (0-7)
        controller->sendMessage(msg);
      }

      return PROCESSED;

    case OPC_RQNPN:
      // RQNPN message -- request parameter by index number
      // index 0 = number of params available;
      // respond with PARAN

      if (nn == module_config->nodeNum)
      {
        byte paran = msg->data[3];

        // DEBUG_SERIAL << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

        if (paran <= controller->_mparams[0])
        {
          paran = msg->data[3];
          controller->sendMessageWithNN(OPC_PARAN, paran, controller->_mparams[paran]);

        } else {
          // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
          controller->sendCMDERR(9);
        }
      }

      return PROCESSED;

    case OPC_SNN:
      // received SNN - set node number
      // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

      if (controller->bModeChanging)
      {
        // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

        // save the NN
        module_config->setNodeNum(nn);

        // respond with NNACK
        controller->sendMessageWithNN(OPC_NNACK);

        // DEBUG_SERIAL << F("> sent NNACK for NN = ") << module_config->nodeNum << endl;

        // we are now in FLiM mode - update the configuration
        controller->setFLiM();

        // DEBUG_SERIAL << F("> current mode = ") << module_config->currentMode << F(", node number = ") << module_config->nodeNum << F(", CANID = ") << module_config->CANID << endl;

      }
      else
      {
        // DEBUG_SERIAL << F("> received SNN but not in transition") << endl;
      }

      return PROCESSED;

    case OPC_CANID:
      // TODO: Belongs to CAN service.

      // CAN -- set CANID
      // DEBUG_SERIAL << F("> CANID for nn = ") << nn << F(" with new CANID = ") << msg->data[3] << endl;

      if (nn == module_config->nodeNum)
      {
        // DEBUG_SERIAL << F("> setting my CANID to ") << msg->data[3] << endl;
        if (msg->data[3] < 1 || msg->data[3] > 99)
        {
          controller->sendCMDERR(7);
        }
        else
        {
          module_config->setCANID(msg->data[3]);
        }
      }

      return PROCESSED;

    case OPC_ENUM:
      // TODO: Belongs to CAN service.

      // received ENUM -- start CAN bus self-enumeration
      // DEBUG_SERIAL << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;
      // DEBUG_SERIAL << F("> my nn = ") << module_config->nodeNum << endl;

      {
        byte remoteCANID = controller->getCANID(msg->id);

        if (nn == module_config->nodeNum && remoteCANID != module_config->CANID && !controller->bCANenum)
        {
          // DEBUG_SERIAL << F("> initiating enumeration") << endl;
          controller->startCANenumeration();
        }
      }

      return PROCESSED;

    case OPC_QNN:
      // this is probably a config recreate -- respond with PNN if we have a node number
      // DEBUG_SERIAL << F("> QNN received, my node number = ") << module_config->nodeNum << endl;

      if (module_config->nodeNum > 0)
      {
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        controller->sendMessageWithNN(OPC_PNN, controller->_mparams[1], controller->_mparams[3], controller->_mparams[8]);
      }

      return PROCESSED;

    case OPC_RQMN:
      // request for node module name, excluding "CAN" prefix
      // sent during module transition, so no node number check
      // DEBUG_SERIAL << F("> RQMN received") << endl;

      // only respond if in transition to FLiM

      // respond with NAME
      if (controller->bModeChanging)
      {
        msg->len = 8;
        msg->data[0] = OPC_NAME;
        memcpy(msg->data + 1, controller->_mname, 7);
        controller->sendMessage(msg);
      }

      return PROCESSED;

    default:
      return NOT_PROCESSED;
  }
}

}