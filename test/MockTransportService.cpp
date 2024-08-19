//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "MockTransportService.h"
#include "Controller.h"

void MockTransportService::process(const VLCB::Action *action)
{
  if (!incoming_messages.empty())
  {
    VLCB::Action incomingAction = {VLCB::ACT_MESSAGE_IN, incoming_messages.front()};
    controller->putAction(incomingAction);
    incoming_messages.pop_front();
  }

  if (action == nullptr)
  {
    return;
  }

  switch (action->actionType)
  {
    case VLCB::ACT_MESSAGE_OUT:
      sent_messages.push_back(action->vlcbMessage);
      break;
      
    default:
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
