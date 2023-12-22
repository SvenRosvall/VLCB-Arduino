// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>                // for definition of byte datatype

namespace VLCB
{

// Interface for persistent storage. Used by Configuration class.
class Storage
{
public:
  virtual void begin() = 0;

  virtual byte read(unsigned int eeaddress) = 0;
  virtual void write(unsigned int eeaddress, byte data) = 0;
  virtual byte readBytes(unsigned int eeaddress, byte nbytes, byte dest[]) = 0;
  virtual void writeBytes(unsigned int eeaddress, const byte src[], byte numbytes) = 0;
  virtual void reset() = 0;
};

extern Storage * createDefaultStorageForPlatform();

}
