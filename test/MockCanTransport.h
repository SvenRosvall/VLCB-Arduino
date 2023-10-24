//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include <deque>
#include <vector>
#include "Transport.h"
#include "Controller.h"
#include "CanTransport.h"
#include "ACAN2515.h"

// This is to replace the hardware layer. It uses the CanTransport class for CAN processing.
class MockCanTransport : public VLCB::CanTransport
{
public:
  virtual bool available() override;
  virtual CANMessage getNextCanMessage() override;
  virtual bool sendMessage(VLCB::CANFrame *msg, bool rtr, bool ext, byte priority) override;
  virtual void reset() override;

  void setNextMessage(CANMessage msg);
  void clearMessages();

  std::deque<CANMessage> incoming_messages;
  std::vector<VLCB::CANFrame> sent_messages;
};
