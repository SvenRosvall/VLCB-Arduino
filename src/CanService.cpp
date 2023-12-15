//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <Streaming.h>
#include "CanService.h"
#include "Controller.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

void CanService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
  canTransport->setController(cntrl);
}

void CanService::startCANenumeration(bool fromENUM)
{
  canTransport->startCANenumeration(fromENUM);
}

void CanService::process(const Command *cmd)
{
  canTransport->process();

  if (cmd == nullptr)
  {
    return;
  }

  switch (cmd->commandType)
  {
    case CMD_MESSAGE_OUT:
      canTransport->sendMessage(&cmd->vlcbMessage);
      break;

    case CMD_MESSAGE_IN:
      handleCanServiceMessage(&cmd->vlcbMessage);
    break;
  }
}

void CanService::process(UserInterface::RequestedAction requestedAction)
{
  canTransport->process(requestedAction);
}

void CanService::handleCanServiceMessage(const VlcbMessage *msg)
{
  unsigned int opc = msg->data[0];
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {
    case OPC_CANID:
      // CAN -- set CANID
      handleSetCANID(msg, nn);
      break;

    case OPC_ENUM:
      // received ENUM -- start CAN bus self-enumeration
      handleEnumeration(nn);
      break;
  }
}

void CanService::handleSetCANID(const VlcbMessage *msg, unsigned int nn)
{
  // DEBUG_SERIAL << F("> CANID for nn = ") << nn << F(" with new CANID = ") << msg->data[3] << endl;

  if (nn == module_config->nodeNum)
  {
    // DEBUG_SERIAL << F("> setting my CANID to ") << msg->data[3] << endl;
    byte newCANID = msg->data[3];
    if (newCANID < 1 || newCANID > 99)
    {
      controller->sendCMDERR(CMDERR_INV_EN_IDX);
      controller->sendGRSP(OPC_CANID, getServiceID(), CMDERR_INV_EN_IDX);
    }
    else
    {
      module_config->setCANID(newCANID);
      controller->sendWRACK();
      controller->sendGRSP(OPC_CANID, getServiceID(), GRSP_OK);
    }
  }
}

void CanService::handleEnumeration(unsigned int nn)
{
  // DEBUG_SERIAL << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;
  // DEBUG_SERIAL << F("> my nn = ") << module_config->nodeNum << endl;

  if (nn == module_config->nodeNum)
  {
    // DEBUG_SERIAL << F("> initiating enumeration") << endl;
    startCANenumeration(true);
  }
}

}