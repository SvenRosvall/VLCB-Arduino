//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockStorage.h"
#include "Configuration.h"

MockStorage::MockStorage()
  : eeprom(1000, 0xFF)
{}

void MockStorage::begin()
{

}

byte MockStorage::read(unsigned int eeaddress)
{
  return eeprom[eeaddress];
}

void MockStorage::write(unsigned int eeaddress, byte data)
{
  eeprom[eeaddress] = data;
}

byte MockStorage::readBytes(unsigned int eeaddress, byte nbytes, byte dest[])
{
  return 0;
}

void MockStorage::writeBytes(unsigned int eeaddress, const byte src[], byte numbytes)
{
  for (byte i = 0; i < numbytes; i++)
  {
    eeprom[eeaddress + i] = src[i];
  }
}

void MockStorage::reset()
{

}

namespace VLCB
{
Storage * createDefaultStorageForPlatform()
{
  static MockStorage storage;
  return &storage;
}

Configuration config(createDefaultStorageForPlatform());
}