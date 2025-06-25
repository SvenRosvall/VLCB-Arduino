//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <Streaming.h>
#include "NodeVariableService.h"
#include "Controller.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

void NodeVariableService::process(const Action *action)
{
  if (action != nullptr && action->actionType == ACT_MESSAGE_IN)
  {
    handleMessage(&action->vlcbMessage);
  }
}

void NodeVariableService::handleMessage(const VlcbMessage *msg)
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);

  switch (opc)
  {
    case OPC_NVRD:
      // received NVRD -- read NV by index
      handleReadNV(msg, nn);
      break;

    case OPC_NVSET:
      // received NVSET -- set NV by index
      handleSetNV(msg, nn);
      break;

    case OPC_NVSETRD:
      // received NVSETRD -- set NV by index and read
      handleSetAndReadNV(msg, nn);
      break;
  }
}

void NodeVariableService::handleReadNV(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  if (msg->len < 4)
  {
    controller->sendGRSP(OPC_NVRD, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte nvindex = msg->data[3];
  Configuration *module_config = controller->getModuleConfig();
  if (nvindex > module_config->EE_NUM_NVS)
  {
    controller->sendGRSP(OPC_NVRD, getServiceID(), CMDERR_INV_NV_IDX);
    controller->sendCMDERR(CMDERR_INV_NV_IDX);
    return;
  }

  if (nvindex == 0)
  {
    controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->EE_NUM_NVS);
    if (!controller->getModuleConfig()->fcuCompatible)
    {
      for (int i = 1; i <= module_config->EE_NUM_NVS; ++i)
      {
        controller->sendMessageWithNN(OPC_NVANS, i, module_config->readNV(i));
      }
    }
  }
  else
  {
    // respond with NVANS
    controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->readNV(nvindex));
  }
}

void NodeVariableService::handleSetNV(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  // DEBUG_SERIAL << F("> received NVSET for nn = ") << nn << endl;
  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_NVSET, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  Configuration *module_config = controller->getModuleConfig();
  if (msg->data[3] > module_config->EE_NUM_NVS)
  {
    controller->sendGRSP(OPC_NVSET, getServiceID(), CMDERR_INV_NV_IDX);
    controller->sendCMDERR(CMDERR_INV_NV_IDX);
  }
  else
  {
    // update EEPROM for this NV -- NVs are indexed from 1, not zero
    module_config->writeNV(msg->data[3], msg->data[4]);
    // respond with WRACK
    controller->sendWRACK();
    // DEBUG_SERIAL << F("> set NV ok") << endl;
  }
}

void NodeVariableService::handleSetAndReadNV(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  // DEBUG_SERIAL << F("> received NVSETRD for nn = ") << nn << endl;
  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_NVSETRD, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte nvindex = msg->data[3];
  Configuration *module_config = controller->getModuleConfig();
  if (nvindex > module_config->EE_NUM_NVS)
  {
    controller->sendGRSP(OPC_NVSETRD, getServiceID(), CMDERR_INV_NV_IDX);
    controller->sendCMDERR(CMDERR_INV_NV_IDX);
    return;
  }

  // update EEPROM for this NV -- NVs are indexed from 1, not zero
  module_config->writeNV(msg->data[3], msg->data[4]);

  // respond with NVANS
  controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->readNV(nvindex));
  // DEBUG_SERIAL << F("> set NV ok") << endl;
}

}