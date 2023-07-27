#pragma once

#include <Arduino.h>                // for definition of byte datatype

namespace VLCB
{

// Interface for persistent storage. Used by Configuration class.
class Storage
{
public:
  virtual void begin() = 0;

  virtual byte readEEPROM(unsigned int eeaddress) = 0;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) = 0;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) = 0;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) = 0;
  virtual void resetEEPROM(void) = 0;
};

extern Storage * createDefaultStorageForPlatform();

}