//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Service.h>

namespace VLCB
{
class Controller;
class Transport;
}

// This replaces the CanTransport for tests that don't really need any CAN specifics.
class MockTransportService : public VLCB::Service
{
public:
  MockTransportService(VLCB::Transport * trpt) : canTransport(trpt) {}
  virtual void setController(VLCB::Controller *cntrl) override;

  virtual byte getServiceID() override { return 0; }
  virtual byte getServiceVersionID() override { return 1; }

  virtual void process(const VLCB::Command * cmd) override;
  
private:
  VLCB::Transport * canTransport;
};
