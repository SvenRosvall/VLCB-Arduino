// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>
#include "UserInterface.h"

namespace VLCB
{

class Controller;
struct Command;
struct VlcbMessage;

enum Processed
{
  NOT_PROCESSED=0,
  PROCESSED=1
};

class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual void begin() {}
  virtual byte getServiceID() = 0;
  virtual byte getServiceVersionID() = 0;

  virtual void process(const Command * cmd) {}
  
  // Keep these for services that have not yet implemented the process(Command).
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) { return NOT_PROCESSED; };
};

}
