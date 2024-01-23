//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Arduino.h>
#include "Transport.h"
#include "ACAN2515.h"

namespace VLCB
{

// Interface for CAN transports 
class CanTransport : public Transport
{
public:
  virtual bool available() = 0;
  virtual CANMessage getNextCanMessage() = 0;
  virtual bool sendCanMessage(CANMessage *msg) = 0;
};

}