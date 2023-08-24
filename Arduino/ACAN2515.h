//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include <cstdint>

#include "SPI.h"

struct ACAN2515Settings
{
  enum Mode {NormalMode};
  ACAN2515Settings(unsigned long, unsigned int);

  Mode mRequestedMode;
  int mReceiveBufferSize;
  int mTransmitBuffer0Size;
  int mTransmitBuffer1Size;
  int mTransmitBuffer2Size;
};

struct CANMessage
{
  uint32_t id;
  uint8_t len;
  bool rtr;
  bool ext;
  uint8_t data[8];
};

struct ACAN2515
{
  ACAN2515(uint8_t i, SPIClass & aClass, uint8_t i1);
  unsigned short begin(const ACAN2515Settings & settings, void (*isr)());
  void isr();
  void poll();
  bool available();
  void receive(CANMessage & message);
  bool tryToSend(const CANMessage & message);
  void end();
  uint16_t receiveBufferPeakCount();
  uint16_t receiveBufferSize();
  uint16_t transmitBufferPeakCount(uint8_t index);
  uint16_t transmitBufferSize(uint8_t index);
  uint8_t errorFlagRegister();
  uint16_t transmitBufferCount(uint8_t index);
  uint16_t receiveBufferCount();
};
