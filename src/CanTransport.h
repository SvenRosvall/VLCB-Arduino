//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Arduino.h>
#include "Transport.h"
#include "ACAN2515.h"
#include "UserInterface.h"

namespace VLCB
{

class CanTransport : public Transport
{
public:
  void setController(Controller *ctrl) override { this->controller = ctrl; }

  void process(UserInterface::RequestedAction requestedAction);

  virtual CANFrame getNextMessage() override;
  virtual CANMessage getNextCanMessage() = 0;

  virtual bool sendMessage(CANFrame *msg) override;
  bool sendRtrMessage();
  virtual bool sendCanMessage(CANMessage *msg) = 0;

  void startCANenumeration(bool fromENUM = false);

protected: // TODO: CAN2515 needs access to controller during refactoring.
  Controller *controller;

private:
  void checkCANenumTimout();
  byte findFreeCanId();

  bool enumeration_required = false;
  bool bCANenum = false;
  bool startedFromEnumMessage = false;
  unsigned long CANenumTime;
  byte enum_responses[16];     // 128 bits for storing CAN ID enumeration results

};

}