#pragma once

#include "Storage.h"

class DueEepromEmulationStorage : public Storage
{
public:
#ifndef __SAM38E__
// If not SAM38E then this file shall be compilable but this class shall not be instantiatable.
  DueEepromEmulationStorage() = delete;
#endif
  void begin();

  virtual byte readEEPROM(unsigned int eeaddress) override;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) override;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) override;
  virtual void resetEEPROM(void) override;

private:
  byte getChipEEPROMVal(unsigned int eeaddress);
  void setChipEEPROMVal(unsigned int eeaddress, byte val);

#ifdef __SAM38E__
  DueFlashStorage dueFlashStorage;
#endif
};
