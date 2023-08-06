// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "MinimumNodeService.h"
#include "Controller.h"
#include <cbusdefs.h>

namespace VLCB
{

enum MnsOpCodes
{
  MNS_OP_RQNN = OPC_RQNN,
  MNS_OP_SNN = OPC_SNN,
  MNS_OP_NNACK = OPC_NNACK,
  MNS_OP_NNREL = OPC_NNREL,
  MNS_OP_QNN = OPC_QNN,
  MNS_OP_PNN = OPC_PNN,
  MNS_OP_RQNP = OPC_RQNP,
  MNS_OP_PARAMS = OPC_PARAMS,
  MNS_OP_RQMN = OPC_RQMN,
  MNS_OP_NAME = OPC_NAME,
  MNS_OP_RQNPN = OPC_RQNPN,
  MNS_OP_PARAN = OPC_PARAN,
  MNS_OP_CMDERR = OPC_CMDERR,
  MNS_OP_GRSP = 0xAF,
  MNS_OP_RDGN = 0x87,
  MNS_OP_DGN = 0xC7,
  MNS_OP_HEARTB = 0xAB,
  MNS_OP_RQSD = 0x78,
  MNS_OP_SD = 0x8C,
  MNS_OP_ESD = 0xE7,
  MNS_OP_MODE = 0x76,
  MNS_OP_SQU = 0x66,
  MNS_OP_NNRST = OPC_NNRST,
  MNS_OP_NNRSM = OPC_NNRSM
};

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
// * RDGN
// * RQSD - Done
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

    case MNS_OP_RQNP:
      // RQNP message - request for node parameters -- does not contain a NN or EN, so only respond if we
      // are in transition to FLiM
      // DEBUG_SERIAL << F("> RQNP -- request for node params during FLiM transition for NN = ") << nn << endl;

      // only respond if we are in transition to FLiM mode
      if (controller->bModeChanging)
      {
        // DEBUG_SERIAL << F("> responding to RQNP with PARAMS") << endl;

        // respond with PARAMS message
        msg->len = 8;
        msg->data[0] = MNS_OP_PARAMS;    // opcode
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

    case MNS_OP_RQNPN:
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
          controller->sendMessageWithNN(MNS_OP_PARAN, paran, controller->_mparams[paran]);

        } else {
          // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
          controller->sendCMDERR(9);
        }
      }

      return PROCESSED;

    case MNS_OP_SNN:
      // received SNN - set node number
      // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

      if (controller->bModeChanging)
      {
        // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

        // save the NN
        module_config->setNodeNum(nn);

        // respond with NNACK
        controller->sendMessageWithNN(MNS_OP_NNACK);

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

    case MNS_OP_QNN:
      // this is probably a config recreate -- respond with PNN if we have a node number
      // DEBUG_SERIAL << F("> QNN received, my node number = ") << module_config->nodeNum << endl;

      if (module_config->nodeNum > 0)
      {
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        controller->sendMessageWithNN(MNS_OP_PNN, controller->_mparams[1], controller->_mparams[3], controller->_mparams[8]);
      }

      return PROCESSED;

    case MNS_OP_RQMN:
      // request for node module name, excluding "CAN" prefix
      // sent during module transition, so no node number check
      // DEBUG_SERIAL << F("> RQMN received") << endl;

      // only respond if in transition to FLiM

      // respond with NAME
      if (controller->bModeChanging)
      {
        msg->len = 8;
        msg->data[0] = MNS_OP_NAME;
        memcpy(msg->data + 1, controller->_mname, 7);
        controller->sendMessage(msg);
      }

      return PROCESSED;

    case MNS_OP_RQSD:
      // Request Service Definitions.

      if (nn == module_config->nodeNum)
      {
        byte serviceIndex = msg->data[3];
        if (serviceIndex == 0)
        {
          controller->sendMessageWithNN(MNS_OP_SD, 0, 0, controller->services.size());
          byte svcIndex = 0;
          for (auto svc : controller->services)
          {
            // TODO: Need to space out these messages, put in a queue or use a TimedResponse structure.
            controller->sendMessageWithNN(MNS_OP_SD, ++svcIndex, svc->getServiceID(), svc->getServiceVersionID());
          }
        }
        else
        {
          byte svcIndex = 0;
          Service * theService = NULL;
          for (auto svc : controller->services)
          {
            if (++svcIndex == serviceIndex)
            {
              theService = svc;
              break;
            }
          }
          if (theService)
          {
            controller->sendMessageWithNN(MNS_OP_ESD, serviceIndex, theService->getServiceID(), 0, 0, 0);
          }
          else
          {
            // Couldn't find the service.
            controller->sendCMDERR(GRSP_INVALID_PARAMETER);
            // NOTE: error code 9 is really for parameters. But there isn't any better for CMDERR.
            controller->sendGRSP(MNS_OP_RQSD, getServiceID(), GRSP_INVALID_SERVICE);
          }
        }
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        controller->sendMessageWithNN(MNS_OP_PNN, controller->_mparams[1], controller->_mparams[3], controller->_mparams[8]);
      }

      return PROCESSED;

    default:
      return NOT_PROCESSED;
  }
}

}