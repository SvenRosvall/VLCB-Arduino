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

void NodeVariableService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

Processed NodeVariableService::handleMessage(unsigned int opc, VlcbMessage *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {
    case OPC_NVRD:
      // received NVRD -- read NV by index
      return handleReadNV(msg, nn);

    case OPC_NVSET:
      // received NVSET -- set NV by index
      return handleSetNV(msg, nn);

    case OPC_NVSETRD:
      // received NVSETRD -- set NV by index and read
      return handleSetAndReadNV(msg, nn);

    default:
      return NOT_PROCESSED;
  }
}

Processed NodeVariableService::handleReadNV(const VlcbMessage *msg, unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (msg->len < 4)
    {
      controller->sendGRSP(OPC_NVRD, getServiceID(), CMDERR_INV_CMD);
      return PROCESSED;
    }

    byte nvindex = msg->data[3];
    if (nvindex > module_config->EE_NUM_NVS)
    {
      controller->sendGRSP(OPC_NVRD, getServiceID(), CMDERR_INV_NV_IDX);
      controller->sendCMDERR(CMDERR_INV_NV_IDX);
    }
    else if (nvindex == 0)
    {
      controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->EE_NUM_NVS);

      for (int i = 1; i <= module_config->EE_NUM_NVS; ++i)
      {
        controller->sendMessageWithNN(OPC_NVANS, i, module_config->readNV(i));
      }
    }
    else
    {
      // respond with NVANS
      controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->readNV(nvindex));
    }
  }

  return PROCESSED;
}

Processed NodeVariableService::handleSetNV(const VlcbMessage *msg, unsigned int nn)
{
  // DEBUG_SERIAL << F("> received NVSET for nn = ") << nn << endl;

  if (nn == module_config->nodeNum)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_NVSET, getServiceID(), CMDERR_INV_CMD);
      return PROCESSED;
    }

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

  return PROCESSED;
}

Processed NodeVariableService::handleSetAndReadNV(const VlcbMessage *msg, unsigned int nn)
{
  // DEBUG_SERIAL << F("> received NVSETRD for nn = ") << nn << endl;

  if (nn == module_config->nodeNum)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_NVSETRD, getServiceID(), CMDERR_INV_CMD);
      return PROCESSED;
    }

    byte nvindex = msg->data[3];
    if (nvindex > module_config->EE_NUM_NVS)
    {
      controller->sendGRSP(OPC_NVSETRD, getServiceID(), CMDERR_INV_NV_IDX);
      controller->sendCMDERR(CMDERR_INV_NV_IDX);
    }
    else
    {
      // update EEPROM for this NV -- NVs are indexed from 1, not zero
      module_config->writeNV(msg->data[3], msg->data[4]);

      // respond with NVANS
      controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->readNV(nvindex));
      // DEBUG_SERIAL << F("> set NV ok") << endl;
    }
  }

  return PROCESSED;
}

}