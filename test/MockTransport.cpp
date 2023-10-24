//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include <Controller.h>
#include "MockTransport.h"

void MockTransport::setController(VLCB::Controller *ctrl)
{
  this->controller = ctrl;
}

bool MockTransport::available()
{
  return !incoming_messages.empty();
}

VLCB::CANFrame MockTransport::getNextMessage()
{
  VLCB::CANFrame frame = incoming_messages.front();
  incoming_messages.pop_front();
  return frame;
}

bool MockTransport::sendMessage(VLCB::CANFrame *msg, bool rtr, bool ext, byte priority)
{
  sent_messages.push_back(*msg);
  return true;
}

void MockTransport::reset()
{

}

void MockTransport::setNextMessage(VLCB::CANFrame msg)
{
  incoming_messages.push_back(msg);
}

void MockTransport::clearMessages()
{
  incoming_messages.clear();
  sent_messages.clear();
}
