// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Storage.h"
#ifdef __SAM3X8E__
#include <DueFlashStorage.h>
#endif


namespace VLCB
{

class DueEepromEmulationStorage : public Storage
{
public:
#ifndef __SAM3X8E__
// If not SAM3X8E then this file shall be compilable but this class shall not be instantiatable and will not compile if used anyway.
  DueEepromEmulationStorage() = delete;
#endif
  virtual void begin() override;

  virtual byte read(unsigned int eeaddress) override;
  virtual byte readBytes(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void write(unsigned int eeaddress, byte data) override;
  virtual void writeBytes(unsigned int eeaddress, const byte src[], byte numbytes) override;
  virtual void reset() override;

private:
  byte getChipEEPROMVal(unsigned int eeaddress);
  void setChipEEPROMVal(unsigned int eeaddress, byte val);

#ifdef __SAM3X8E__
  DueFlashStorage dueFlashStorage;
#endif
};

}