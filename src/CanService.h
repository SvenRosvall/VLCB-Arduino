//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "Service.h"
#include "UserInterface.h"

namespace VLCB
{

class Configuration;

class CanService : public Service
{

public:

  virtual void setController(Controller *cntrl) override;
  virtual byte getServiceID() override { return 3; }
  virtual byte getServiceVersionID() override { return 1; }

  virtual void process(UserInterface::RequestedAction requestedAction) override;
  virtual Processed handleRawMessage(CANFrame *msg) override;
  Processed handleMessage(unsigned int opc, CANFrame *msg) override;

  void startCANenumeration();

private:

  Controller *controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.

  bool enumeration_required;
  bool bCANenum;
  unsigned long CANenumTime;
  byte enum_responses[16];     // 128 bits for storing CAN ID enumeration results

  static byte getCANID(unsigned long header);
  void checkCANenumTimout();
  byte findFreeCanId();
};

}