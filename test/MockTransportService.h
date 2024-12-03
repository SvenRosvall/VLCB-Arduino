//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Service.h>
#include <deque>
#include <vector>
#include <Controller.h>

namespace VLCB
{
class Controller;
}

// This replaces the CanTransport for tests that don't really need any CAN specifics.
class MockTransportService : public VLCB::Service
{
public:
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_CAN; }
  virtual byte getServiceVersionID() const override { return 1; }

  virtual void process(const VLCB::Action * action) override;

  // Mock support to inject messages to be received and inspect sent messages
  void setNextMessage(VLCB::VlcbMessage msg);
  void clearMessages();

  std::deque<VLCB::VlcbMessage> incoming_messages;
  std::vector<VLCB::VlcbMessage> sent_messages;
};
