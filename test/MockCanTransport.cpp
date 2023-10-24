//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include <Controller.h>
#include "MockCanTransport.h"

bool MockCanTransport::available()
{
  return !incoming_messages.empty();
}

CANMessage MockCanTransport::getNextCanMessage()
{
  CANMessage msg = incoming_messages.front();
  incoming_messages.pop_front();
  return msg;
}

bool MockCanTransport::sendMessage(VLCB::CANFrame *msg, bool rtr, bool ext, byte priority)
{
  // Update the message the same way as CAN2515 does.
  msg->id = controller->getModuleCANID();
  msg->rtr = rtr;
  msg->ext = ext;

  sent_messages.push_back(*msg);
  return true;
}

void MockCanTransport::reset()
{

}

void MockCanTransport::setNextMessage(CANMessage msg)
{
  incoming_messages.push_back(msg);
}

void MockCanTransport::clearMessages()
{
  incoming_messages.clear();
  sent_messages.clear();
}
