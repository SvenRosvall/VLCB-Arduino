// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "EepromExternalStorage.h"

#include <Wire.h>

namespace VLCB
{

EepromExternalStorage::EepromExternalStorage(byte address)
{
  external_address = address;
  I2Cbus = &Wire;
}

EepromExternalStorage::EepromExternalStorage(byte address, TwoWire *bus)
{
  external_address = address;
  I2Cbus = bus;
}

void EepromExternalStorage::begin()
{
  // from Configuration::setEEPROMtype()

  I2Cbus->begin();
  I2Cbus->beginTransmission(external_address);
  byte result = I2Cbus->endTransmission();

  //if (result == 0) {
    // DEBUG_SERIAL << F("> external EEPROM selected") << endl;
  //} else {
    // DEBUG_SERIAL << F("> external EEPROM not found") << endl;
  //}
}


//
/// read a single byte from EEPROM
//
byte EepromExternalStorage::read(unsigned int eeaddress)
{
  byte rdata = 0;

  // DEBUG_SERIAL << F("> read, addr = ") << eeaddress << endl;

  I2Cbus->beginTransmission(external_address);
  I2Cbus->write((int)(eeaddress >> 8));    // MSB
  I2Cbus->write((int)(eeaddress & 0xFF));  // LSB
  int r = I2Cbus->endTransmission();

  if (r < 0) {
    // DEBUG_SERIAL << F("> read: I2C write error = ") << r << endl;
  }

  I2Cbus->requestFrom((int)external_address, (int)1);

  if (I2Cbus->available()) {
    rdata = I2Cbus->read();
  }

  return rdata;
}


//
/// read a number of bytes from EEPROM
/// external EEPROM must use 16-bit addresses !!
//
byte EepromExternalStorage::readBytes(unsigned int eeaddress, byte nbytes, byte dest[])
{
  I2Cbus->beginTransmission(external_address);
  I2Cbus->write((int)(eeaddress >> 8));    // MSB
  I2Cbus->write((int)(eeaddress & 0xFF));  // LSB
  int r = I2Cbus->endTransmission();

  if (r < 0) {
    // DEBUG_SERIAL << F("> readBytes: I2C write error = ") << r << endl;
  }

  I2Cbus->requestFrom((int)external_address, (int)nbytes);

  byte count = 0;
  while (I2Cbus->available() && count < nbytes) {
    dest[count++] = I2Cbus->read();
  }

  // DEBUG_SERIAL << F("> readBytes: read ") << count << F(" bytes from EEPROM in ") << micros() - t1 << F("us") << endl;

  return count;
}


//
/// write a byte
//
void EepromExternalStorage::write(unsigned int eeaddress, byte data)
{
  // DEBUG_SERIAL << F("> write, addr = ") << eeaddress << F(", data = ") << data << endl;
  I2Cbus->beginTransmission(external_address);
  I2Cbus->write((int)(eeaddress >> 8)); // MSB
  I2Cbus->write((int)(eeaddress & 0xFF)); // LSB
  I2Cbus->write(data);
  int r = I2Cbus->endTransmission();
  delay(5);

  if (r < 0) {
    // DEBUG_SERIAL << F("> write: I2C write error = ") << r << endl;
  }
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//
void EepromExternalStorage::writeBytes(unsigned int eeaddress, const byte src[], byte numbytes)
{
  // *** TODO *** handle greater than 32 bytes -> the Arduino I2C write buffer size
  // max write = EEPROM pagesize - 64 bytes

  I2Cbus->beginTransmission(external_address);
  I2Cbus->write((int) (eeaddress >> 8));   // MSB
  I2Cbus->write((int) (eeaddress & 0xFF)); // LSB

  for (byte i = 0; i < numbytes; i++)
  {
    I2Cbus->write(src[i]);
  }

  int r = I2Cbus->endTransmission();
  delay(5);

  if (r < 0)
  {
    // DEBUG_SERIAL << F("> writeBytes: I2C write error = ") << r << endl;
  }
}

//
/// clear all event data in external EEPROM chip
//
void EepromExternalStorage::reset()
{
  // DEBUG_SERIAL << F("> clearing data from external EEPROM ...") << endl;

  for (unsigned int addr = 10; addr < 4096; addr++)
  {
    write(addr, 0xff);
  }
}

}