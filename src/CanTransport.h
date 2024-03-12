//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Arduino.h>
#include "Transport.h"

namespace VLCB
{

template <typename T>
struct CANFrame
{
};

// Interface for CAN transports 
template <typename T>
class CanTransport : public Transport
{
public:
  virtual bool available() = 0;
  virtual CANFrame<T> getNextCanFrame() = 0;
  virtual bool sendCanFrame(CANFrame<T> *msg) = 0;
};

}