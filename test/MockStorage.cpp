//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockStorage.h"
#include "Configuration.h"

void MockStorage::begin()
{

}

byte MockStorage::readEEPROM(unsigned int eeaddress)
{
  return 0;
}

void MockStorage::writeEEPROM(unsigned int eeaddress, byte data)
{

}

byte MockStorage::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte *dest)
{
  return 0;
}

void MockStorage::writeBytesEEPROM(unsigned int eeaddress, byte *src, byte numbytes)
{

}

void MockStorage::resetEEPROM()
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