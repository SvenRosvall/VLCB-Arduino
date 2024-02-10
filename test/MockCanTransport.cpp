//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockCanTransport.h"

bool MockCanTransport::available()
{
  return !incoming_frames.empty();
}

VLCB::CANFrame MockCanTransport::getNextCanFrame()
{
  VLCB::CANFrame msg = incoming_frames.front();
  incoming_frames.pop_front();
  return msg;
}

bool MockCanTransport::sendCanFrame(uint32_t id, bool rtr, bool ext, const VLCB::VlcbMessage *msg)
{
  VLCB::CANFrame frame;
  frame.id = id;
  frame.rtr = rtr;
  frame.ext = ext;
  if (msg == nullptr)
  {
    frame.len = 0;
  }
  else
  {
    frame.len = msg->len;
    memcpy(frame.data, msg->data, msg->len);
  }
  sent_frames.push_back(frame);
  return true;
}

void MockCanTransport::reset()
{

}

void MockCanTransport::setNextMessage(VLCB::CANFrame frame)
{
  incoming_frames.push_back(frame);
}

void MockCanTransport::clearMessages()
{
  incoming_frames.clear();
  sent_frames.clear();
}
