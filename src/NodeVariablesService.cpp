//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <Streaming.h>
#include "NodeVariablesService.h"
#include "Controller.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

void NodeVariablesService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

Processed NodeVariablesService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

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

    default:
      return NOT_PROCESSED;
  }
}

}