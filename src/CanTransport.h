//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Arduino.h>
#include "Transport.h"

namespace VLCB
{

struct VlcbMessage;

struct CANFrame
{
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8];
};

struct CreateCanFrameCallback
{
  virtual void handleIncomingCanFrame(uint32_t id, bool rtr, bool ext, uint8_t len, uint8_t data[8]) = 0;
};

// Interface for CAN transports 
class CanTransport : public Transport
{
public:
  virtual bool available() = 0;
  virtual void getNextCanFrame(CreateCanFrameCallback *callback) = 0;
  virtual bool sendCanFrame(uint32_t id, bool rtr, bool ext, const VlcbMessage *msg) = 0;
};

}