// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>

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
  virtual Processed handleMessage(unsigned int opc, CANFrame *msg) = 0;
};

}

