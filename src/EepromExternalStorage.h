// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Storage.h"

#include <Arduino.h>                // for definition of byte datatype
#include <Wire.h>

namespace VLCB
{

class EepromExternalStorage : public Storage
{
public:
  EepromExternalStorage(byte address);
  EepromExternalStorage(byte address, TwoWire *bus);
  virtual void begin() override;

  virtual byte readEEPROM(unsigned int eeaddress) override;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) override;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) override;
  virtual void resetEEPROM() override;

private:
  byte external_address;
  TwoWire *I2Cbus;
};

}