// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <SPI.h>

namespace VLCB
{

const int DEFAULT_PRIORITY = 0xB;     // default Controller messages priority. 1011 = 2|3 = normal/low

class Controller;
class CANFrame;

// Interface for sending and receiving Controller messages.
class Transport
{
public:
  virtual void setController(Controller * ctrl) { }
  virtual bool available() = 0;
  virtual CANFrame getNextMessage() = 0;
  virtual bool sendMessage(CANFrame *msg) = 0;
  virtual void reset() = 0;
};

}
