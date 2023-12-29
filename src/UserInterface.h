// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>
#include "vlcbdefs.hpp"

namespace VLCB
{

class UserInterface
{
public:

  enum RequestedAction
  {
    NONE,
    CHANGE_MODE,
    RENEGOTIATE,
    ENUMERATION
  };

  virtual void run() = 0;
  virtual void indicateActivity() = 0;
  virtual void indicateMode(VlcbModeParams mode) = 0;
  virtual RequestedAction checkRequestedAction() = 0;
};

}
