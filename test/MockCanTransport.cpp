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

VLCB::CANFrame<MockCanFrame> MockCanTransport::getNextCanFrame()
{
  VLCB::CANFrame<MockCanFrame> msg = incoming_frames.front();
  incoming_frames.pop_front();
  return msg;
}

bool MockCanTransport::sendCanFrame(VLCB::CANFrame<MockCanFrame> *frame)
{
  sent_frames.push_back(*frame);
  return true;
}

void MockCanTransport::reset()
{

}

void MockCanTransport::setNextMessage(VLCB::CANFrame<MockCanFrame> frame)
{
  incoming_frames.push_back(frame);
}

void MockCanTransport::clearMessages()
{
  incoming_frames.clear();
  sent_frames.clear();
}
