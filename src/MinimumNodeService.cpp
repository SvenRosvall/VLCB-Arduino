// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "MinimumNodeService.h"
#include "Controller.h"

namespace VLCB
{

void copyName(uint8_t *dest, const char *name)
{
  int i = 0;
  for (; i < 7 ; ++i)
  {
    if (name[i] == '\0')
    {
      // If the string is shorter than 7 chars, then don't copy the null and continue below to pad with spaces. 
      break;
    }
    dest[i] = name[i];
  }
  for (; i < 7 ; ++i)
  {
    dest[i] = ' ';
  }
}

void MinimumNodeService::begin()
{
  //Initialise instantMode
  instantMode = controller->getModuleConfig()->currentMode;
  noHeartbeat = !controller->getModuleConfig()->heartbeat;
  controller->indicateMode(instantMode);
  //DEBUG_SERIAL << F("> instant MODE initialise as: ") << instantMode << endl;
  
  notFcuCompatible = !controller->getModuleConfig()->fcuCompatible;
}

//
/// initiate the transition from Uninitialised to Normal mode
//
void MinimumNodeService::initSetupFromUninitialised()
{
  // DEBUG_SERIAL << F("> initiating Normal negotation") << endl;

  // enumerate the CAN bus to allocate a free CAN ID
  Action action = {ACT_START_CAN_ENUMERATION, false };
  controller->putAction(action);

  initSetupCommon();
}

void MinimumNodeService::initSetupCommon()
{
  instantMode = MODE_SETUP;
  controller->indicateMode(MODE_SETUP);

  // send RQNN message with current NN, which may be zero if a virgin/Uninitialised node
  controller->sendMessageWithNN(OPC_RQNN);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << controller->getModuleConfig()->nodeNum << endl;
}

void MinimumNodeService::setNormal(unsigned int nn)
{
  // DEBUG_SERIAL << F("> set Normal") << endl;
  instantMode = MODE_NORMAL;
  controller->getModuleConfig()->setModuleNormalMode(nn);
  controller->indicateMode(MODE_NORMAL);
}

//
/// set module to Uninitialised mode
//
void MinimumNodeService::setUninitialised()
{
  // DEBUG_SERIAL << F("> set Uninitialised") << endl;
  if (controller->getModuleConfig()->nodeNum != 0  && instantMode == MODE_NORMAL)
  {
    controller->sendMessageWithNN(OPC_NNREL);  // release node number first
  }
  instantMode = MODE_UNINITIALISED;
  controller->getModuleConfig()->setModuleUninitializedMode();
  controller->indicateMode(MODE_UNINITIALISED);
}

//
/// request new node number
//
void MinimumNodeService::initSetupFromNormal()
{
  // DEBUG_SERIAL << F("> reverting to Setup mode") << endl;
  initSetupCommon();
}

void MinimumNodeService::heartbeat()
{
  if ((controller->getModuleConfig()->currentMode == MODE_NORMAL) && !noHeartbeat && instantMode != MODE_SETUP)
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
        switch (instantMode)
        {
        case MODE_UNINITIALISED:
          initSetupFromUninitialised();
          break;
           
        case MODE_NORMAL:
        case MODE_SETUP:
          setUninitialised();
          break;
           
        default:
          break;
        }
        break;
      
      case ACT_RENEGOTIATE:
        switch (instantMode)
        {
        case MODE_UNINITIALISED:
          break;
          
        case MODE_SETUP:
          // Cancel setup and revert to previous mode.
          instantMode = controller->getModuleConfig()->currentMode;
          controller->indicateMode(instantMode);

          if (controller->getModuleConfig()->nodeNum != 0)
          {
            // Revert to previous NN   
            controller->sendMessageWithNN(OPC_NNACK);
          }
            
          break;
           
        case MODE_NORMAL:
          initSetupFromNormal();
          break;
           
        default:
          break;
        }
        break;
      
      case ACT_MESSAGE_IN:
        handleMessage(&action->vlcbMessage);
        break;

      default:
          break;
    }
  }

  heartbeat();
}

void MinimumNodeService::handleMessage(const VlcbMessage *msg)
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  Configuration *module_config = controller->getModuleConfig();

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
      // If we are in setup, abort (MNS Spec 3.1.1)

      if (instantMode == MODE_SETUP)
      {
        instantMode = module_config->currentMode;
        controller->indicateMode(module_config->currentMode);
        controller->messageActedOn();
      }
      break;

    case OPC_QNN:
      // 0D - this is probably a config recreate -- respond with PNN if we have a node number
      // DEBUG_SERIAL << F("> QNN received, my node number = ") << controller->getModuleConfig()->nodeNum << endl;

      if (module_config->nodeNum > 0)
      {
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        controller->sendMessageWithNN(OPC_PNN, controller->getParam(PAR_MANU), controller->getParam(PAR_MTYP), controller->getParam(PAR_FLAGS));
        controller->messageActedOn();
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
        copyName(response.data + 1, module_config->getModuleName());
        controller->sendMessage(&response);
        controller->messageActedOn();
      }
      break;

    case OPC_RQSD:
      // 78 - Request Service Definitions.
      handleRequestServiceDefinitions(msg, nn);
      break;

    case OPC_MODE:
      // 76 - Set Operating Mode
      handleModeMessage(msg, nn);
      break;

    case OPC_NNRSM:
      //4F - reset to manufacturer's defaults 
      if (isThisNodeNumber(nn))
      {        
        controller->sendMessageWithNN(OPC_NNREL);  // release node number first
        module_config->resetModule();
        controller->messageActedOn();
      }
      break;
      
    case OPC_NNRST:
      //5E - software reset
      if (isThisNodeNumber(nn))
      {
        module_config->reboot();
        controller->messageActedOn();
      }
      break;
  }
}

void MinimumNodeService::handleRequestNodeParameters()
{
  // DEBUG_SERIAL << F("> RQNP -- request for node params during Normal transition for NN = ") << nn << endl;

  // only respond if we are in transition to Normal mode
  if (instantMode != MODE_SETUP)
  {
    return;
  }

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
  controller->messageActedOn();
}

void MinimumNodeService::handleRequestNodeParameter(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  if (msg->len < 4)
  {
    controller->sendGRSP(OPC_RQNPN, getServiceID(), CMDERR_INV_CMD);
  }
  else
  {
    byte paran = msg->data[3];

    //DEBUG_SERIAL << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

    if ((paran == 0) && notFcuCompatible)
    {
      for (byte i = 0; i <= controller->getParam(PAR_NUM); i++)
      {
        controller->sendMessageWithNN(OPC_PARAN, i, controller->getParam((VlcbParams) i));
      }
    }
    else if (paran <= controller->getParam(PAR_NUM))
    {
      controller->sendMessageWithNN(OPC_PARAN, paran, controller->getParam((VlcbParams) paran));
    }
    else
    {
      // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
      controller->sendCMDERR(CMDERR_INV_PARAM_IDX);
      controller->sendGRSP(OPC_RQNPN, getServiceID(), CMDERR_INV_PARAM_IDX);
    }
  }
  controller->messageActedOn();
}

void MinimumNodeService::handleSetNodeNumber(const VlcbMessage *msg, unsigned int nn)
{      // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

  if (instantMode != MODE_SETUP)
  {
    return;
  }

  if (msg->len < 3)
  {
    controller->sendGRSP(OPC_SNN, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

  // we are now in Normal mode - update the configuration
  setNormal(nn);
  // DEBUG_SERIAL << F("> current mode = ") << controller->getModuleConfig()->currentMode << F(", node number = ") << controller->getModuleConfig()->nodeNum << F(", CANID = ") << controller->getModuleConfig()->CANID << endl;

  // respond with NNACK
  controller->sendMessageWithNN(OPC_NNACK);
  // DEBUG_SERIAL << F("> sent NNACK for NN = ") << controller->getModuleConfig()->nodeNum << endl;
  
  diagNodeNumberChanged();
  controller->messageActedOn();
}

static int countServices(const VLCB::ArrayHolder<Service *> &services)
{
  int count = 0;
  for (auto svc : services)
  {
    if (svc->getServiceID() > 0)
    {
      ++count;
    }
  }
  return count;
}

void MinimumNodeService::handleRequestServiceDefinitions(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  if (msg->len < 4)
  {
    controller->sendGRSP(OPC_RQSD, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte serviceIndex = msg->data[3];
  if (serviceIndex == 0)
  {
    // Request for summary of services. First a service count
    int serviceCount = countServices(controller->getServices());
    controller->sendMessageWithNN(OPC_SD, 0, 0, serviceCount);

    // and then details of each service.
    byte svcIndex = 0;
    for (auto svc: controller->getServices())
    {
      ++svcIndex;
      if (svc->getServiceID() > 0)
      {
        // TODO: Need to space out these messages, put in a queue or use a TimedResponse structure.
        controller->sendMessageWithNN(OPC_SD, svcIndex, svc->getServiceID(), svc->getServiceVersionID());
      }
    }
  }
  else if (serviceIndex <= controller->getServices().size())
  {
    // Request for details of a single service.
    Service *theService = controller->getServices()[serviceIndex - 1];
    if (theService->getServiceID() == 0)
    {
      controller->sendGRSP(OPC_RQSD, getServiceID(), GRSP_INVALID_SERVICE);
    }
    else
    {
      controller->sendMessageWithNN(OPC_ESD, serviceIndex, theService->getServiceID(), 0, 0, 0);
    }
  }
  else
  {
    // DEBUG_SERIAL << "RQSD wrong svcIx=" << serviceIndex << endl;
    // Couldn't find the service.
    controller->sendGRSP(OPC_RQSD, getServiceID(), GRSP_INVALID_SERVICE);
  }
  controller->messageActedOn();
}

void MinimumNodeService::handleModeMessage(const VlcbMessage *msg, unsigned int nn)
{
  //DEBUG_SERIAL << F("> MODE -- request op-code received for NN = ") << nn << endl;
  if (!isThisNodeNumber(nn))
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
      switch (instantMode)
      {
        case MODE_SETUP:
        case MODE_NORMAL:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
          setUninitialised();
          break;
      
        default:
          break;
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

        case MODE_SETUP:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_INVALID_MODE);
          break;

        default:
          break;
      }
      break;
      
    case MODE_NORMAL:
      // Request Normal. Only OK if we are already in Normal mode.
      switch (instantMode)
      {
        case MODE_NORMAL:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_OK);
          break;

        case MODE_SETUP:
          controller->sendGRSP(OPC_MODE, getServiceID(), GRSP_INVALID_MODE);
          break;

        default:
          break;
      }
      break;

    case MODE_HEARTBEAT_ON:
    case MODE_HEARTBEAT_OFF:
      noHeartbeat = (requestedMode == MODE_HEARTBEAT_OFF);
      controller->getModuleConfig()->setHeartbeat(!noHeartbeat);
      break;
      
    case MODE_FCU_COMPATABILITY_ON:
    case MODE_FCU_COMPATABILITY_OFF:
      notFcuCompatible = (requestedMode == MODE_FCU_COMPATABILITY_OFF);
      controller->getModuleConfig()->setFcuCompatability(!notFcuCompatible);
      break;
      
    default:
      if (instantMode != MODE_NORMAL)
      { 
        controller->sendGRSP(OPC_MODE, getServiceID(), CMDERR_INV_CMD);
      }
      return;
  }
  controller->messageActedOn();
}

void MinimumNodeService::setSetupMode()
{
  instantMode = MODE_SETUP;
}

}