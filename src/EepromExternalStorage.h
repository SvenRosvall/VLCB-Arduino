#pragma once

#include "Storage.h"

#include <Arduino.h>                // for definition of byte datatype
#include <Wire.h>

class EepromExternalStorage : public Storage
{
  EepromExternalStorage();
  void setExtEEPROMAddress(byte address, TwoWire *bus);
  void begin();

  virtual byte readEEPROM(unsigned int eeaddress) override;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) override;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) override;
  virtual void resetEEPROM(void) override;

private:
  byte external_address;
  TwoWire *I2Cbus;
};
