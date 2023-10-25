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
  virtual VLCB::VlcbMessage getNextMessage() override;
  virtual bool sendMessage(VLCB::VlcbMessage *msg) override;
  virtual void reset() override;
  void setNextMessage(VLCB::VlcbMessage msg);
  void clearMessages();

  std::deque<VLCB::VlcbMessage> incoming_messages;
  std::vector<VLCB::VlcbMessage> sent_messages;
  
private:
  VLCB::Controller * controller;
};
