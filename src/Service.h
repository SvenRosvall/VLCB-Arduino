// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>

namespace VLCB
{

class Controller;
struct Command;

class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual void begin() {}
  virtual byte getServiceID() = 0;
  virtual byte getServiceVersionID() = 0;

  virtual void process(const Command * cmd) = 0;
};

}
