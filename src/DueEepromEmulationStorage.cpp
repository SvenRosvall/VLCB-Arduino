// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "DueEepromEmulationStorage.h"

#ifdef __SAM3X8E__
#include <DueFlashStorage.h>
#endif

namespace VLCB
{

void DueEepromEmulationStorage::begin()
{
}


//
/// read a single byte from EEPROM
//
byte DueEepromEmulationStorage::read(unsigned int eeaddress)
{
  byte rdata = getChipEEPROMVal(eeaddress);
  return rdata;
}

//
/// read a number of bytes from EEPROM
/// external EEPROM must use 16-bit addresses !!
//
byte DueEepromEmulationStorage::readBytes(unsigned int eeaddress, byte nbytes, byte dest[])
{
  byte count;

  for (count = 0; count < nbytes; count++)
  {
    dest[count] = getChipEEPROMVal(eeaddress + count);
  }

  return count;
}

byte DueEepromEmulationStorage::getChipEEPROMVal(unsigned int eeaddress)
{
#ifdef __SAM3X8E__
  return dueFlashStorage.read(eeaddress);
#endif
}


//
/// write a byte
//
void DueEepromEmulationStorage::write(unsigned int eeaddress, byte data)
{
  // DEBUG_SERIAL << F("> write, addr = ") << eeaddress << F(", data = ") << data << endl;

  setChipEEPROMVal(eeaddress, data);
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//
void DueEepromEmulationStorage::writeBytes(unsigned int eeaddress, byte src[], byte numbytes)
{
  for (byte i = 0; i < numbytes; i++) 
  {
    setChipEEPROMVal(eeaddress + i, src[i]);
  }
}

//
/// architecture-neutral methods to read and write the microcontroller's on-chip EEPROM (or emulation)
/// as EEPROM.h is not available for all, and a post-write commit may or may not be required
//
void DueEepromEmulationStorage::setChipEEPROMVal(unsigned int eeaddress, byte val)
{
#ifdef __SAM3X8E__
  dueFlashStorage.write(eeaddress, val);
#endif
}

//
/// clear all event data in external EEPROM chip
//
void DueEepromEmulationStorage::reset()
{

}

}