#pragma once

#include "Storage.h"

namespace VLCB
{

class EepromInternalStorage : public Storage
{
public:
  virtual void begin() override;

  virtual byte readEEPROM(unsigned int eeaddress) override;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) override;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) override;
  virtual void resetEEPROM(void) override;

private:
  byte getChipEEPROMVal(unsigned int eeaddress);
  void setChipEEPROMVal(unsigned int eeaddress, byte val);
};

}