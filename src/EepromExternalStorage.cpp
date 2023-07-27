#include "EepromExternalStorage.h"

#include <Wire.h>

namespace VLCB
{

EepromExternalStorage::EepromExternalStorage()
{
  I2Cbus = &Wire;
}

void EepromExternalStorage::setExtEEPROMAddress(byte address, TwoWire *bus)
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

byte EepromExternalStorage::readEEPROM(unsigned int eeaddress) {

  byte rdata = 0;

  // DEBUG_SERIAL << F("> readEEPROM, addr = ") << eeaddress << endl;

  I2Cbus->beginTransmission(external_address);
  I2Cbus->write((int)(eeaddress >> 8));    // MSB
  I2Cbus->write((int)(eeaddress & 0xFF));  // LSB
  int r = I2Cbus->endTransmission();

  if (r < 0) {
    // DEBUG_SERIAL << F("> readEEPROM: I2C write error = ") << r << endl;
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

byte EepromExternalStorage::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) {

    I2Cbus->beginTransmission(external_address);
    I2Cbus->write((int)(eeaddress >> 8));    // MSB
    I2Cbus->write((int)(eeaddress & 0xFF));  // LSB
    int r = I2Cbus->endTransmission();

    if (r < 0) {
      // DEBUG_SERIAL << F("> readBytesEEPROM: I2C write error = ") << r << endl;
    }

    I2Cbus->requestFrom((int)external_address, (int)nbytes);

    byte count = 0;
    while (I2Cbus->available() && count < nbytes) {
      dest[count++] = I2Cbus->read();
    }

    // DEBUG_SERIAL << F("> readBytesEEPROM: read ") << count << F(" bytes from EEPROM in ") << micros() - t1 << F("us") << endl;

  return count;
}


//
/// write a byte
//

void EepromExternalStorage::writeEEPROM(unsigned int eeaddress, byte data) {

  // DEBUG_SERIAL << F("> writeEEPROM, addr = ") << eeaddress << F(", data = ") << data << endl;
    I2Cbus->beginTransmission(external_address);
    I2Cbus->write((int)(eeaddress >> 8)); // MSB
    I2Cbus->write((int)(eeaddress & 0xFF)); // LSB
    I2Cbus->write(data);
    int r = I2Cbus->endTransmission();
    delay(5);

    if (r < 0) {
      // DEBUG_SERIAL << F("> writeEEPROM: I2C write error = ") << r << endl;
    }
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//

void EepromExternalStorage::writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) {

  // *** TODO *** handle greater than 32 bytes -> the Arduino I2C write buffer size
  // max write = EEPROM pagesize - 64 bytes

    I2Cbus->beginTransmission(external_address);
    I2Cbus->write((int)(eeaddress >> 8));   // MSB
    I2Cbus->write((int)(eeaddress & 0xFF)); // LSB

    for (byte i = 0; i < numbytes; i++) {
      I2Cbus->write(src[i]);
    }

    int r = I2Cbus->endTransmission();
    delay(5);

    if (r < 0) {
      // DEBUG_SERIAL << F("> writeBytesEEPROM: I2C write error = ") << r << endl;
    }
}

//
/// clear all event data in external EEPROM chip
//

void EepromExternalStorage::resetEEPROM(void) {

    // DEBUG_SERIAL << F("> clearing data from external EEPROM ...") << endl;

    for (unsigned int addr = 10; addr < 4096; addr++) {
      writeEEPROM(addr, 0xff);
    }
}

}