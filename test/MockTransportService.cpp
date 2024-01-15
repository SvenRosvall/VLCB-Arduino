//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "MockTransportService.h"
#include "Controller.h"

void MockTransportService::setController(VLCB::Controller *cntrl)
{
  this->controller = cntrl;
}

void MockTransportService::process(const VLCB::Command *cmd)
{
  if (!incoming_messages.empty())
  {
    VLCB::Command cmd = {VLCB::CMD_MESSAGE_IN, incoming_messages.front()};
    controller->putCommand(cmd);
    incoming_messages.pop_front();
  }

  if (cmd == nullptr)
  {
    return;
  }

  switch (cmd->commandType)
  {
    case VLCB::CMD_MESSAGE_OUT:
      sent_messages.push_back(cmd->vlcbMessage);
      break;
  }
}

void MockTransportService::setNextMessage(VLCB::VlcbMessage msg)
{
  incoming_messages.push_back(msg);
}

void MockTransportService::clearMessages()
{
  incoming_messages.clear();
  sent_messages.clear();
}
