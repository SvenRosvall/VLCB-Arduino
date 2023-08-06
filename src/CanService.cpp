//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "CanService.h"
#include "Controller.h"
#include <cbusdefs.h>

namespace VLCB
{

enum CanOpCodes
{
  CAN_OP_ENUM = OPC_ENUM,
  CAN_OP_CANID = OPC_CANID
};

void CanService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

// TODO: This list is used while implementing MNS. Remove once done.
// CAN shall implement these opcodes in incoming requests
// * RDGN
// * CANID - Here
// * ENUM - Here

Processed CanService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {

    case CAN_OP_CANID:
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

    case CAN_OP_ENUM:
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

    default:
      return NOT_PROCESSED;
  }
}

}