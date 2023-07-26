#include "DueEepromEmulationStorage.h"

#ifdef __SAM38E__
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

byte DueEepromEmulationStorage::readEEPROM(unsigned int eeaddress) {

  byte rdata = getChipEEPROMVal(eeaddress);

  return rdata;
}

//
/// read a number of bytes from EEPROM
/// external EEPROM must use 16-bit addresses !!
//

byte DueEepromEmulationStorage::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) {

  byte count;

    for (count = 0; count < nbytes; count++) {
      dest[count] = getChipEEPROMVal(eeaddress + count);
    }

  return count;
}

byte DueEepromEmulationStorage::getChipEEPROMVal(unsigned int eeaddress) {

#ifdef __SAM38E__
  return dueFlashStorage.read(eeaddress);
#endif
}


//
/// write a byte
//

void DueEepromEmulationStorage::writeEEPROM(unsigned int eeaddress, byte data) {

  // DEBUG_SERIAL << F("> writeEEPROM, addr = ") << eeaddress << F(", data = ") << data << endl;

    setChipEEPROMVal(eeaddress, data);
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//

void DueEepromEmulationStorage::writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) {

    for (byte i = 0; i < numbytes; i++) {
      setChipEEPROMVal(eeaddress + i, src[i]);
    }
}
//
/// architecture-neutral methods to read and write the microcontroller's on-chip EEPROM (or emulation)
/// as EEPROM.h is not available for all, and a post-write commit may or may not be required
//

void DueEepromEmulationStorage::setChipEEPROMVal(unsigned int eeaddress, byte val) {

#ifdef __SAM38E__
  dueFlashStorage.write(eeaddress, val);
#endif
}

//
/// clear all event data in external EEPROM chip
//

void DueEepromEmulationStorage::resetEEPROM(void) {

}

}