// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "EepromInternalStorage.h"

#include <EEPROM.h>

namespace VLCB
{

void EepromInternalStorage::begin()
{
  // these devices require an explicit begin with the desired emulated size

#if defined ESP32 || defined ESP8266
  EEPROM.begin(EE_EVENTS_START + (EE_MAX_EVENTS * EE_BYTES_PER_EVENT));
#endif

#ifdef ARDUINO_ARCH_RP2040
  EEPROM.begin(4096);
#endif
}


//
/// read a single byte from EEPROM
//
byte EepromInternalStorage::readEEPROM(unsigned int eeaddress)
{
  byte rdata = getChipEEPROMVal(eeaddress);

  return rdata;
}

//
/// read a number of bytes from EEPROM
/// external EEPROM must use 16-bit addresses !!
//
byte EepromInternalStorage::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[])
{
  byte count;
    for (count = 0; count < nbytes; count++) {
      dest[count] = getChipEEPROMVal(eeaddress + count);
    }

  return count;
}

byte EepromInternalStorage::getChipEEPROMVal(unsigned int eeaddress)
{
  return EEPROM.read(eeaddress);
}


//
/// write a byte
//
void EepromInternalStorage::writeEEPROM(unsigned int eeaddress, byte data)
{
// DEBUG_SERIAL << F("> writeEEPROM, addr = ") << eeaddress << F(", data = ") << data << endl;

  setChipEEPROMVal(eeaddress, data);
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//
void EepromInternalStorage::writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes)
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
void EepromInternalStorage::setChipEEPROMVal(unsigned int eeaddress, byte val)
{
  EEPROM.write(eeaddress, val);

#if defined ESP32 || defined ESP8266 || defined ARDUINO_ARCH_RP2040
  EEPROM.commit();
#endif
}

//
/// clear all event data in external EEPROM chip
//
void EepromInternalStorage::resetEEPROM()
{
  // Note: There was no code for resetting internal EEPROM. Instead this reset was done in resetModule()

  // DEBUG_SERIAL << F("> clearing data from external EEPROM ...") << endl;

  for (unsigned int addr = 10; addr < 4096; addr++)
  {
    writeEEPROM(addr, 0xff);
  }
}

}