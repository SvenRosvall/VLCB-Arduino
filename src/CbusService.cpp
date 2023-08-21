// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "CbusService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void CbusService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

Processed CbusService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];
  // DEBUG_SERIAL << ">CbusSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {

  case OPC_NVRD:
    // received NVRD -- read NV by index
    if (nn == module_config->nodeNum) {

      byte nvindex = msg->data[3];
      if (nvindex > module_config->EE_NUM_NVS)
      {
        controller->sendCMDERR(10);
      }
      else
      {
        // respond with NVANS
        controller->sendMessageWithNN(OPC_NVANS, nvindex, module_config->readNV(nvindex));
      }
    }

    return PROCESSED;

  case OPC_NVSET:
    // received NVSET -- set NV by index
    // DEBUG_SERIAL << F("> received NVSET for nn = ") << nn << endl;

    if (nn == module_config->nodeNum)
    {
      if (msg->data[3] > module_config->EE_NUM_NVS)
      {
        controller->sendCMDERR(10);
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

  case OPC_BOOT:
    // boot mode
    return PROCESSED;

  case OPC_RSTAT:
    // command station status -- not applicable to accessory modules
    return PROCESSED;

  // case OPC_ARST:
  // system reset ... this is not what I thought it meant !
  // module_config->reboot();
  // return PROCESSED;

  default:
    // unknown or unhandled OPC
    // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
    return NOT_PROCESSED;
  }
}

}