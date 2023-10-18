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

class MockTransport : public VLCB::Transport
{
public:
  virtual void setController(VLCB::Controller *ctrl) override;
  virtual bool available() override;
  virtual VLCB::CANFrame getNextMessage() override;
  virtual bool sendMessage(VLCB::CANFrame *msg, bool rtr, bool ext, byte priority) override;
  virtual void reset() override;
  void setNextMessage(VLCB::CANFrame msg);
  void clearMessages();

  std::deque<VLCB::CANFrame> incoming_messages;
  std::vector<VLCB::CANFrame> sent_messages;
  
private:
  VLCB::Controller * controller;
};
