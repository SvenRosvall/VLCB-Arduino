// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "MinimumNodeService.h"
#include "Controller.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

void MinimumNodeService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

//
/// initiate the transition from Uninitialised to Normal mode
//
void MinimumNodeService::initNormal()
{
  // DEBUG_SERIAL << F("> initiating Normal negotation") << endl;

  controller->indicateMode(MODE_SETUP);

  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/Uninitialised node
  controller->sendMessageWithNN(OPC_RQNN);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

void MinimumNodeService::setNormal()
{
  // DEBUG_SERIAL << F("> set Normal") << endl;
  bModeChanging = false;
  module_config->setModuleMode(MODE_NORMAL);
  controller->indicateMode(MODE_NORMAL);

  // enumerate the CAN bus to allocate a free CAN ID
  controller->startCANenumeration();
}

//
/// set module to Uninitialised mode
//
void MinimumNodeService::setUninitialised()
{
  // DEBUG_SERIAL << F("> set Uninitialised") << endl;
  bModeChanging = false;
  module_config->setNodeNum(0);
  module_config->setModuleMode(MODE_UNINITIALISED);
  module_config->setCANID(0);

  controller->indicateMode(MODE_UNINITIALISED);
}

//
/// revert from Normal to Uninitialised mode
//
void MinimumNodeService::revertUninitialised()
{

  // DEBUG_SERIAL << F("> reverting to Uninitialised mode") << endl;

  // send NNREL message

  controller->sendMessageWithNN(OPC_NNREL);
  setUninitialised();
}

//
/// change or re-confirm node number
//

void MinimumNodeService::renegotiate()
{
  initNormal();
}

//
/// check 30 sec timeout for Uninitialised/Normal negotiation with FCU
//
void MinimumNodeService::checkModeChangeTimeout()
{
  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // Revert to previous mode.
    // DEBUG_SERIAL << F("> timeout expired, currentMode = ") << currentMode << F(", mode change = ") << bModeChanging << endl;
    controller->indicateMode(module_config->currentMode);
    bModeChanging = false;
  }
}

void MinimumNodeService::heartbeat()
{
  if ((module_config->currentMode == MODE_NORMAL) && !noHeartbeat)
  {
    if ((millis() - lastHeartbeat) > heartRate)
    {
    //  DEBUG_SERIAL << F("> HeartBeat = ") << heartbeatSequence << endl;
      controller->sendMessageWithNN(OPC_HEARTB, heartbeatSequence, 0, 0);  // 0 to be replaced by diagnostic status      
      heartbeatSequence++;
      lastHeartbeat = millis();
    }
  }  
}

//
/// MinimumNode Service processing procedure
//

void MinimumNodeService::process(UserInterface::RequestedAction requestedAction)
{
   switch (requestedAction)
  {
    case UserInterface::CHANGE_MODE:
      // initiate mode change
      //Serial << "Controller::process() - changing mode, current mode=" << module_config->currentMode << endl;
      if (!module_config->currentMode)
      {
        initNormal();
      }
      else
      {
        revertUninitialised();
      }
      break;

    case UserInterface::RENEGOTIATE:
      //Serial << "Controller::process() - renegotiate" << endl;
      renegotiate();
      break;

    default:
      break;
  }
  
  checkModeChangeTimeout();
  heartbeat();
}

// TODO: This list is used while implementing MNS. Remove once done.
// MNS shall implement these opcodes in incoming requests
// * SNN - Done
// * QNN - Done
// * RQNP - Done
// * RQMN - Done
// * RQNPN - Done
// * RDGN - Request Diagnostic Data (0x87)
// * RQSD - Done
// * MODE - Set Operating Mode (0x76)
// * SQU - ????
// * NNRST - Done
// * NNRSM - Done

Processed MinimumNodeService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {

    case OPC_RQNP:
      // RQNP message - request for node parameters -- does not contain a NN or EN, so only respond if we
      // are in transition to Normal
      // DEBUG_SERIAL << F("> RQNP -- request for node params during Normal transition for NN = ") << nn << endl;

      // only respond if we are in transition to Normal mode
      if (bModeChanging)
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

      if (bModeChanging)
      {
        // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

        // save the NN
        module_config->setNodeNum(nn);

        // respond with NNACK
        controller->sendMessageWithNN(OPC_NNACK);

        // DEBUG_SERIAL << F("> sent NNACK for NN = ") << module_config->nodeNum << endl;

        // we are now in Normal mode - update the configuration
        setNormal();

        // DEBUG_SERIAL << F("> current mode = ") << module_config->currentMode << F(", node number = ") << module_config->nodeNum << F(", CANID = ") << module_config->CANID << endl;

      }
      else
      {
        // DEBUG_SERIAL << F("> received SNN but not in transition") << endl;
      }

      return PROCESSED;
      
    case OPC_RQNN:
      // Another module has entered setup.
      // If we are in setup, abort (MNS Spec 3.2.1)
      
      if (bModeChanging)
      {
        bModeChanging = false;
        controller->indicateMode(module_config->currentMode);
      }
        
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

      // only respond if in transition to Normal

      // respond with NAME
      if (bModeChanging)
      {
        msg->len = 8;
        msg->data[0] = OPC_NAME;
        memcpy(msg->data + 1, controller->_mname, 7);
        controller->sendMessage(msg);
      }

      return PROCESSED;

    case OPC_RQSD:
      // Request Service Definitions.

      if (nn == module_config->nodeNum)
      {
        byte serviceIndex = msg->data[3];
        if (serviceIndex == 0)
        {
          controller->sendMessageWithNN(OPC_SD, 0, 0, controller->services.size());
          byte svcIndex = 0;
          for (auto svc : controller->services)
          {
            // TODO: Need to space out these messages, put in a queue or use a TimedResponse structure.
            controller->sendMessageWithNN(OPC_SD, ++svcIndex, svc->getServiceID(), svc->getServiceVersionID());
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
            controller->sendMessageWithNN(OPC_ESD, serviceIndex, theService->getServiceID(), 0, 0, 0);
          }
          else
          {
            // Couldn't find the service.
            controller->sendCMDERR(CMDERR_INV_EN_IDX);
            // NOTE: error code 9 is really for parameters. But there isn't any better for CMDERR.
            controller->sendGRSP(OPC_RQSD, getServiceID(), GRSP_INVALID_SERVICE);
          }
        }
      }

      return PROCESSED;
      
    case OPC_RDGN:
      // Request Diagnostic Data
      
      return PROCESSED;
      
    case OPC_MODE:
      // Set Operating Mode
      
      return PROCESSED;
      
    case OPC_NNRSM:
      //reset to manufacturer's defaults 
      if (nn == module_config->nodeNum)
      {        
        controller->sendMessageWithNN(OPC_NNREL);  // release node number first
        module_config->resetModule();        
      }
      return PROCESSED;
      
    case OPC_NNRST:
      //software reset
      if (nn == module_config->nodeNum)
      {
        module_config->reboot();
      }
      return PROCESSED;

    default:
      return NOT_PROCESSED;
  }
}

}