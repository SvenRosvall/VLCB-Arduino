// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>

namespace VLCB
{

class Controller;

class UserInterface
{
public:

  virtual void run() = 0;
  virtual void indicateResetting() =0;
  virtual void indicateResetDone() = 0;
  virtual void indicateActivity() = 0;
  virtual void indicateMode(byte mode) = 0;
  virtual bool resetRequested() = 0;

  void setController(Controller *ctrl)
  {
    this->controller = ctrl;
  }

protected:
  Controller * controller;
};

}
