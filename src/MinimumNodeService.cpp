// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "MinimumNodeService.h"
#include "Controller.h"

namespace VLCB
{

void MinimumNodeService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

void MinimumNodeService::begin()
{
  //Initialise instantMode
  instantMode = module_config->currentMode;
  noHeartbeat = !module_config->heartbeat;
  controller->indicateMode(instantMode);
  //DEBUG_SERIAL << F("> instant MODE initialise as: ") << instantMode << endl;
}

//
/// initiate the transition from Uninitialised to Normal mode
//
void MinimumNodeService::initSetup()
{
  // DEBUG_SERIAL << F("> initiating Normal negotation") << endl;

  instantMode = MODE_SETUP;
  controller->indicateMode(MODE_SETUP);

  timeOutTimer = millis();
  
  // enumerate the CAN bus to allocate a free CAN ID
  Action action = {ACT_START_CAN_ENUMERATION, false };
  controller->putAction(action);

  // send RQNN message with current NN, which may be zero if a virgin/Uninitialised node
  controller->sendMessageWithNN(OPC_RQNN);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

void MinimumNodeService::setNormal(unsigned int nn)
{
  // DEBUG_SERIAL << F("> set Normal") << endl;
  requestingNewNN = false;
  instantMode = MODE_NORMAL;
  module_config->setModuleNormalMode(nn);
  controller->indicateMode(MODE_NORMAL);
}

//
/// set module to Uninitialised mode
//
void MinimumNodeService::setUninitialised()
{
  // DEBUG_SERIAL << F("> set Uninitialised") << endl;
  requestingNewNN = false;
  instantMode = MODE_UNINITIALISED;
  module_config->setModuleUninitializedMode();
  module_config->setCANID(0);

  controller->indicateMode(MODE_UNINITIALISED);
}

//
/// revert from Normal to Uninitialised mode
//
void MinimumNodeService::initSetupFromNormal()
{
  // DEBUG_SERIAL << F("> reverting to Uninitialised mode") << endl;
  requestingNewNN = true;
  controller->sendMessageWithNN(OPC_NNREL);
  initSetup();
}

//
/// check 30 sec timeout for MODE_CHANGE negotiation with FCU
//
void MinimumNodeService::checkModeChangeTimeout()
{
  if (instantMode == MODE_SETUP && ((millis() - timeOutTimer) >= 30000)) 
  {
    // Revert to previous mode.
    // DEBUG_SERIAL << F("> timeout expired, currentMode = ") << currentMode << F(", mode change = ") << bModeSetup << endl;
    instantMode = module_config->currentMode;
    controller->indicateMode(instantMode);

    if (requestingNewNN)
    {
      // Renegotiating timed out.  Revert to previous NN   
      requestingNewNN = false;
      controller->sendMessageWithNN(OPC_NNACK);
    }
  }
}

void MinimumNodeService::heartbeat()
{
  if ((module_config->currentMode == MODE_NORMAL) && !noHeartbeat && instantMode != MODE_SETUP)
  {
    if ((millis() - lastHeartbeat) > heartRate)
    {
      //DEBUG_SERIAL << F("> HeartBeat = ") << heartbeatSequence << endl;
      controller->sendMessageWithNN(OPC_HEARTB, heartbeatSequence, 0, 0);  // 0 to be replaced by diagnostic status      
      heartbeatSequence++;
      lastHeartbeat = millis();
    }
  }  
}

//
/// MinimumNode Service processing procedure
//

void MinimumNodeService::process(const Action *action)
{
  if (action != nullptr)
  {
    switch (action->actionType)
    {
      case ACT_CHANGE_MODE:
      {
        switch (module_config->currentMode)
        {
        case MODE_UNINITIALISED:
          initSetup();
          break;
           
        case MODE_NORMAL:
          initSetupFromNormal();
          break;
           
        default:
          break;
        }
        break;
      }
      
      case ACT_MESSAGE_IN:
        handleMessage(&action->vlcbMessage);

    }
  }

  checkModeChangeTimeout();
  heartbeat();
}

// TODO: This list is used while implementing MNS. Remove once done.
// MNS shall implement these opcodes in incoming requests
// * RDGN - Request Diagnostic Data (0x87)

void MinimumNodeService::handleMessage(const VlcbMessage *msg)
{
  unsigned int opc = msg->data[0];
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {
    case OPC_RQNP:
      // 10 - RQNP message - request for node parameters -- does not contain a NN or EN, so only respond if we
      // are in transition to Normal
      handleRequestNodeParameters();
      break;

    case OPC_RQNPN:
      // 73 - RQNPN message -- request parameter by index number
      // index 0 = number of params available followed by each parameter as a seperate PARAN
      // respond with PARAN
      handleRequestNodeParameter(msg, nn);
      break;

    case OPC_SNN:
      // 42 - received SNN - set node number
      handleSetNodeNumber(msg, nn);
      break;

    case OPC_RQNN:
      // 50 - Another module has entered setup.
      // If we are in setup, abort (MNS Spec 3.2.1)

      if (instantMode == MODE_SETUP)
      {
        instantMode = module_config->currentMode;
        controller->indicateMode(module_config->currentMode);
      }
      break;

    case OPC_QNN:
      // 0D - this is probably a config recreate -- respond with PNN if we have a node number
      // DEBUG_SERIAL << F("> QNN received, my node number = ") << module_config->nodeNum << endl;

      if (module_config->nodeNum > 0)
      {
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        controller->sendMessageWithNN(OPC_PNN, controller->getParam(PAR_MANU), controller->getParam(PAR_MTYP), controller->getParam(PAR_FLAGS));
      }
      break;

    case OPC_RQMN:
      // 11 - request for node module name, excluding "CAN" prefix
      // sent during module transition, so no node number check
      // DEBUG_SERIAL << F("> RQMN received") << endl;

      // only respond if in transition to Normal, i.e. Setup mode, or in learn mode.

      if (instantMode == MODE_SETUP || (controller->getParam(PAR_FLAGS) & PF_LRN))
      {
        // respond with NAME
        VlcbMessage response;
        response.len = 8;
        response.data[0] = OPC_NAME;
        memcpy(response.data + 1, controller->getModuleName(), 7);
        controller->sendMessage(&response);
      }
      break;

    case OPC_RQSD:
      // 78 - Request Service Definitions.
      handleRequestServiceDefinitions(msg, nn);
      break;

    case OPC_RDGN:
      // 87 - Request Diagnostic Data
      handleRequestDiagnostics(msg, nn);
      break;

    case OPC_MODE:
      // 76 - Set Operating Mode
      handleModeMessage(msg, nn);
      break;

    case OPC_NNRSM:
      //4F - reset to manufacturer's defaults 
      if (nn == module_config->nodeNum)
      {        
        controller->sendMessageWithNN(OPC_NNREL);  // release node number first
        module_config->resetModule();        
      }
      break;
      
    case OPC_NNRST:
      //5E - software reset
      if (nn == module_config->nodeNum)
      {
        module_config->reboot();
      }
      break;
  }
}

void MinimumNodeService::handleRequestNodeParameters()
{
  // DEBUG_SERIAL << F("> RQNP -- request for node params during Normal transition for NN = ") << nn << endl;

  // only respond if we are in transition to Normal mode
  if (instantMode == MODE_SETUP)
  {
    // DEBUG_SERIAL << F("> responding to RQNP with PARAMS") << endl;

    // respond with PARAMS message
    VlcbMessage response;
    response.len = 8;
    response.data[0] = OPC_PARAMS;    // opcode
    response.data[1] = controller->getParam(PAR_MANU);     // manf code -- MERG
    response.data[2] = controller->getParam(PAR_MINVER);     // minor code ver
    response.data[3] = controller->getParam(PAR_MTYP);     // module ident
    response.data[4] = controller->getParam(PAR_EVTNUM);     // number of events
    response.data[5] = controller->getParam(PAR_EVNUM);     // events vars per event
    response.data[6] = controller->getParam(PAR_NVNUM);     // number of NVs
    response.data[7] = controller->getParam(PAR_MAJVER);     // major code ver

    controller->sendMessage(&response);
  }
}

void MinimumNodeService::handleRequestNodeParameter(const VlcbMessage *msg, unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (msg->len < 4)
    {
      controller->sendGRSP(OPC_RQNPN, getServiceID(), CMDERR_INV_CMD);
    }
    else
    {
      byte paran = msg->data[3];

      //DEBUG_SERIAL << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

      if (paran == 0)
      {
        for (byte i = 0; i <= controller->getParam(PAR_NUM); i++)
        {
          controller->sendMessageWithNN(OPC_PARAN, i, controller->getParam(i));
        }
      }
      else if (paran <= controller->getParam(PAR_NUM))
      {
        controller->sendMessageWithNN(OPC_PARAN, paran, controller->getParam(paran));
      }
      else
      {
        // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
        controller->sendCMDERR(CMDERR_INV_PARAM_IDX);
        controller->sendGRSP(OPC_RQNPN, getServiceID(), CMDERR_INV_PARAM_IDX);
      }
    }
  }
}

void MinimumNodeService::handleSetNodeNumber(const VlcbMessage *msg, unsigned int nn)
{      // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

  if (instantMode == MODE_SETUP)
  {
    if (msg->len < 3)
    {
      controller->sendGRSP(OPC_SNN, getServiceID(), CMDERR_INV_CMD);
    }
    else
    {
      // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

      // we are now in Normal mode - update the configuration
      setNormal(nn);
      // DEBUG_SERIAL << F("> current mode = ") << module_config->currentMode << F(", node number = ") << module_config->nodeNum << F(", CANID = ") << module_config->CANID << endl;

      // respond with NNACK
      controller->sendMessageWithNN(OPC_NNACK);
      // DEBUG_SERIAL << F("> sent NNACK for NN = ") << module_config->nodeNum << endl;
    }
  }
}

void MinimumNodeService::handleRequestServiceDefinitions(const VlcbMessage *msg, unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (msg->len < 4)
    {
      controller->sendGRSP(OPC_RQSD, getServiceID(), CMDERR_INV_CMD);
      return;
    }

    byte serviceIndex = msg->data[3];
    if (serviceIndex == 0)
    {
      // Request for summary of services. First a service count
      controller->sendMessageWithNN(OPC_SD, 0, 0, controller->getServices().size());

      // and then details of each service.
      byte svcIndex = 0;
      for (auto svc: controller->getServices())
      {
        // TODO: Need to space out these messages, put in a queue or use a TimedResponse structure.
        controller->sendMessageWithNN(OPC_SD, ++svcIndex, svc->getServiceID(), svc->getServiceVersionID());
      }
    }
    else if (serviceIndex <= controller->getServices().size())
    {
      // Request for details of a single service.
      Service *theService = controller->getServices()[serviceIndex - 1];
      controller->sendMessageWithNN(OPC_ESD, serviceIndex, theService->getServiceID(), 0, 0, 0);
    }
    else
    {
      // Couldn't find the service.
      controller->sendGRSP(OPC_RQSD, getServiceID(), GRSP_INVALID_SERVICE);
    }
  }
}

void MinimumNodeService::handleRequestDiagnostics(const VlcbMessage *msg, unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_RDGN, getServiceID(), CMDERR_INV_CMD);
      return;
    }
    byte serviceIndex = msg->data[3];
    if (serviceIndex <= controller->getServices().size())
    {
      Service *theService = controller->getServices()[serviceIndex - 1];
      byte diagnosticCode = msg->data[4];
      // TODO: more stuff to go in here    
    }
    else
    {
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_SERVICE);
    }
  }
}

void MinimumNodeService::handleModeMessage(const VlcbMessage *msg, unsigned int nn)
{
  //DEBUG_SERIAL << F("> MODE -- request op-code received for NN = ") << nn << endl;
  if (nn != module_config->nodeNum)
  {
    // Not for this module.
    return;
  }

  if (msg->len < 4)
  {
    controller->sendGRSP(OPC_MODE, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte requestedMode = msg->data[3];
  //DEBUG_SERIAL << F("> MODE -- requested = ") << requestedMode << endl;
  //DEBUG_SERIAL << F("> instant MODE  = ") << instantMode << endl;

  switch (requestedMode)
  {
    case MODE_UNINITIALISED:
      // Request factory reset mode
      if (instantMode == MODE_SETUP)
      {
        controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
        setUninitialised();
      }
      else
      {
        controller->sendGRSP(OPC_MODE, getServiceID(), CMDERR_INV_CMD);
      }
      break;

    case MODE_SETUP:
      // Request Setup
      switch (instantMode)
      {
        case MODE_NORMAL:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
          initSetupFromNormal();
          break;
      
        case MODE_UNINITIALISED:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
          initSetup();
          break;

        default:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_INVALID_MODE);
          break;
      }
      break;
      
    case MODE_NORMAL:
      // Request Normal. Only OK if we are already in Normal mode.
      if (instantMode == MODE_NORMAL)
      {
        controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
      }
      else
      {
        controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_INVALID_MODE);
      }
      break;

    case MODE_HEARTBEAT_ON:
      // Turn on Heartbeat
      noHeartbeat = false;
      module_config->setHeartbeat(!noHeartbeat);
      break;

    case MODE_HEARTBEAT_OFF:
      // Turn off Heartbeat
      noHeartbeat = true;
      module_config->setHeartbeat(!noHeartbeat);
      break;
      
    default:
      if (instantMode != MODE_NORMAL)
      { 
        controller->sendGRSP(OPC_MODE, getServiceID(), CMDERR_INV_CMD);
      }
      break;
  }
}

void MinimumNodeService::setSetupMode()
{
  instantMode = MODE_SETUP;
  timeOutTimer = 0;
}

}