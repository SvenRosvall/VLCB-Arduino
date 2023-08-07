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
class CANFrame;

enum Processed
{
  NOT_PROCESSED=0,
  PROCESSED=1
};

class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual byte getServiceID() = 0;
  virtual byte getServiceVersionID() = 0;

  virtual void process(UserInterface::RequestedAction requestedAction) {}
  virtual Processed handleRawMessage(CANFrame *msg) { return NOT_PROCESSED; }
  virtual Processed handleMessage(unsigned int opc, CANFrame *msg) = 0;
};

}

