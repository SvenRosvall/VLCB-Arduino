
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

//
/// CBUS module configuration
/// manages the storage of events and node variables in on-chip or external EEPROM
//

#include <Wire.h>
#include <Streaming.h>
#include <EEPROM.h>

#include "CBUSconfig.h"

// the event hash table
byte evhashtbl[64];
bool hash_collision = false;

//
/// initialise and set default values
//

void CBUSConfig::begin(void) {

    makeEvHashTable();
    loadNVs();
}

//
/// set the EEPROM type for event storage
/// on-chip or external I2C bus device
//

bool CBUSConfig::setEEPROMtype(byte type) {

  bool r;

  if (type == EEPROM_EXTERNAL) {
    Wire.begin();
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    byte r = Wire.endTransmission();

    if (r == 0) {
      eeprom_type = EEPROM_EXTERNAL;
      // Serial << F("> external EEPROM selected") << endl;
      r = true;
    } else {
      // Serial << F("> external EEPROM not found") << endl;
      eeprom_type = EEPROM_INTERNAL;
      r = false;
    }
  } else {
    eeprom_type = EEPROM_INTERNAL;
    // Serial << F("> internal EEPROM selected") << endl;
    r = true;
  }

  return r;
}

//
/// store the FLiM mode
//

void CBUSConfig::setFLiM(bool f) {

  FLiM = f;
  EEPROM[0] = f;
}

//
/// store the CANID
//

void CBUSConfig::setCANID(byte canid) {

  CANID = canid;
  EEPROM[1] = canid;
}

//
/// store the node number
//

void CBUSConfig::setNodeNum(unsigned int nn) {

  nodeNum = nn;
  EEPROM[2] = highByte(nodeNum);
  EEPROM[3] = lowByte(nodeNum);
}

//
/// lookup an event by node number and event number, using the hash table
//

byte CBUSConfig::findExistingEvent(unsigned int nn, unsigned int en) {

  byte tarray[4];
  byte tmphash, i, j;

  // Serial << F("> looking for match with ") << nn << ", " << en << endl;

  tarray[0] = highByte(nn);
  tarray[1] = lowByte(nn);
  tarray[2] = highByte(en);
  tarray[3] = lowByte(en);

  // calc the hash of the incoming event to match
  tmphash = makeHash(tarray);
  // Serial << F("> event hash = ") << tmphash << endl;

  for (i = 0; i < EE_MAX_EVENTS; i++) {

    if (evhashtbl[i] == tmphash) {
      if (!hash_collision) {
        // NN + EN hash matches and there are no hash collisions
        // no further checking is required
        // Serial << F("> unique match found at hash table index = ") << i << endl;
      } else {
        // there is a potential hash collision so we have to check the slower way
        // first, check if this hash appears in the table more than once
        byte matches = 0;
        for (j = 0; j < EE_MAX_EVENTS; j++) {
          if (evhashtbl[j] == tmphash) {
            ++matches;
          }
        }

        if (matches > 1) {
          // one or more collisions for this hash exist, so check the very slow way
          // Serial << F("> this hash matches = ") << matches << endl;

          for (i = 0; i < EE_MAX_EVENTS; i++) {
            if (evhashtbl[i] == tmphash) {
              // check the EEPROM for a match with the incoming NN and EN
              readEvent(i, tarray);
              if ((unsigned int)((tarray[0] << 8) + tarray[1]) == nn && (unsigned int)((tarray[2] << 8) + tarray[3]) == en) {
                // the stored NN and EN match this event, so no further checking is required
                break;
              }
            }
          }
        } else {
          // no collisions for this specific hash, so no further checking is required
          break;
        }
      }

      break;
    }
  }

  // if (i >= EE_MAX_EVENTS) Serial << F("> unable to find matching event") << endl;
  return i;
}

//
/// find an empty EEPROM event slot - the hash table entry == 0
//

byte CBUSConfig::findEventSpace(void) {

  byte evidx;

  for (evidx = 0; evidx < EE_MAX_EVENTS; evidx++) {
    if (evhashtbl[evidx] == 0)  {
      // Serial << F("> found empty location at index = ") << evidx << endl;
      break;
    }
  }

  return evidx;
}

//
/// create a hash from a 4-byte event entry array -- NN + EN
//

byte CBUSConfig::makeHash(byte tarr[]) {

  unsigned char hash;
  unsigned int nn, en;

  // make a hash from a 4-byte NN + EN event

  nn = (tarr[0] << 8) + tarr[1];
  en = (tarr[2] << 8) + tarr[3];

  // need to hash the NN and EN to a uniform distribution across HASH_LENGTH
  hash = nn ^ (nn >> 8);
  hash = 7 * hash + (en ^ (en >> 8));

  // ensure it is within bounds
  hash %= HASH_LENGTH;

  // Serial << F("> makeHash - hash of nn = ") << nn << F(", en = ") << en << F(", = ") << hash << endl;
  return hash;
}

//
/// return an existing EEPROM event as a 4-byte array -- NN + EN
//

void CBUSConfig::readEvent(byte idx, byte tarr[]) {

  // populate the array with the first 4 bytes (NN + EN) of the event entry from the EEPROM
  for (byte i = 0; i < EE_HASH_BYTES; i++) {
    tarr[i] = readEEPROM(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + i);
  }

  // Serial << F("> readEvent - idx = ") << idx << F(", nn = ") << (tarr[0] << 8) + tarr[1] << F(", en = ") << (tarr[2] << 8) + tarr[3] << endl;
  return;
}

//
/// return an event variable (EV) value given the event table index and EV number
//

byte CBUSConfig::getEventEVval(byte idx, byte evnum) {

  return readEEPROM(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + 3 + evnum);
}

//
/// re/create the event hash table
//

void CBUSConfig::makeEvHashTable(void) {

  byte evarray[4];

  // Serial << F("> creating event hash table") << endl;

  for (byte idx = 0; idx < EE_MAX_EVENTS; idx++) {

    readEvent(idx, evarray);

    if (evarray[0] == 0xff) {
      evhashtbl[idx] = 0;
    } else {
      evhashtbl[idx] = makeHash(evarray);
    }
  }

  hash_collision = check_hash_collisions();

  return;
}

//
/// update a single hash table entry -- after a learn or unlearn
//

void CBUSConfig::updateEvHashEntry(byte idx) {

  byte evarray[4];

  // read the first four bytes from EEPROM - NN + EN
  readEvent(idx, evarray);

  // empty slots have first byte set to 0xff
  if (evarray[0] == 0xff) {
    evhashtbl[idx] = 0;
  } else {
    evhashtbl[idx] = makeHash(evarray);
  }

  hash_collision = check_hash_collisions();

  // Serial << F("> updateEvHashEntry for idx = ") << idx << F(", hash = ") << hash << endl;
  return;
}

//
/// clear the hash table
//

void CBUSConfig::clearEvHashTable(void) {

  // zero in the hash table indicates that the corresponding event slot is free
  // Serial << F("> clearEvHashTable - clearing hash table") << endl;

  for (byte i = 0; i < EE_MAX_EVENTS; i++) {
    evhashtbl[i] = 0;
  }

  hash_collision = false;
  return;
}

//
/// print the event hash table
//

void CBUSConfig::printEvHashTable(bool raw) {

  Serial << F("> Event hash table --") << endl;

  for (byte i = 0; i < EE_MAX_EVENTS; i++) {
    if (evhashtbl[i] > 0) {
      if (raw)
        Serial << evhashtbl[i] << endl;
      else
        Serial << F("  -- ") << i << " - " << evhashtbl[i] << endl;
    }
  }

  Serial << endl;
  return;
}

//
/// return the number of stored events
//

byte CBUSConfig::numEvents(void) {

  byte numevents = 0;

  for (byte i = 0; i < EE_MAX_EVENTS; i++) {
    if (evhashtbl[i] != 0) {
      ++numevents;
    }
  }

  return numevents;
}

//
/// return a single hash table entry by index
//

byte CBUSConfig::getEvTableEntry(byte tindex) {

  if (tindex < EE_MAX_EVENTS) {
    return evhashtbl[tindex];
  } else {
    return 0;
  }
}

//
/// read an NV value from local EEPROM
/// note that NVs number from 1, not 0
//

byte CBUSConfig::readNV(byte idx) {

  return (EEPROM[EE_NVS_START + (idx - 1)]);
}

//
/// write an NV value to local EEPROM
/// note that NVs number from 1, not 0
//

void CBUSConfig::writeNV(byte idx, byte val) {

  EEPROM[EE_NVS_START + (idx - 1)] = val;
  return;
}

//
/// read a single byte from EEPROM
//

byte CBUSConfig::readEEPROM(unsigned int eeaddress) {

  byte rdata = 0;
  int r = 0;

  // Serial << F("> readEEPROM, addr = ") << eeaddress << endl;

  if (eeprom_type == EEPROM_EXTERNAL) {

    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((int)(eeaddress >> 8));    // MSB
    Wire.write((int)(eeaddress & 0xFF));  // LSB
    r = Wire.endTransmission();

    if (r < 0) {
      Serial << F("> readEEPROM: I2C write error = ") << r << endl;
    }

    Wire.requestFrom(EEPROM_I2C_ADDR, (uint8_t)1);

    if (Wire.available()) rdata = Wire.read();

  } else {
    rdata = EEPROM[eeaddress];
  }

  return rdata;
}

//
/// read a number of bytes from EEPROM
//

byte CBUSConfig::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) {

  int r = 0;
  byte count = 0;

  if (eeprom_type == EEPROM_EXTERNAL) {

    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((int)(eeaddress >> 8));    // MSB
    Wire.write((int)(eeaddress & 0xFF));  // LSB
    r = Wire.endTransmission();

    if (r < 0) {
      // Serial << F("> readBytesEEPROM: I2C write error = ") << r << endl;
    }

    Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, (uint8_t)nbytes);

    while (Wire.available() && count < nbytes) {
      dest[count++] = Wire.read();
    }

    // Serial << F("> readBytesEEPROM: read ") << count << F(" bytes from EEPROM in ") << micros() - t1 << F("us") << endl;
  } else {
  
    for (count = 0; count < nbytes; count++) {
      dest[count] = EEPROM[eeaddress + count];
    }
  }

  return count;
}

//
/// write a byte
//

void CBUSConfig::writeEEPROM(unsigned int eeaddress, byte data) {

  int r = 0;

  // Serial << F("> writeEEPROM, addr = ") << eeaddress << F(", data = ") << data << endl;

  if (eeprom_type == EEPROM_EXTERNAL) {

    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(data);
    r = Wire.endTransmission();
    delay(5);

    if (r < 0) {
      Serial << F("> writeEEPROM: I2C write error = ") << r << endl;
    }
  } else {
    EEPROM[eeaddress] = data;
  }

  return;
}

//
/// write a number of bytes to EEPROM
//

void CBUSConfig::writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) {

  // *** TODO *** handle greater than 32 bytes -> the Arduino I2C write buffer size
  // max write = EEPROM pagesize - 64 bytes

  int r = 0;

  if (eeprom_type == EEPROM_EXTERNAL) {

    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB

    for (byte i = 0; i < numbytes; i++) {
      Wire.write(src[i]);
    }

    r = Wire.endTransmission();
    delay(5);

    if (r < 0) {
      Serial << F("> writeBytesEEPROM: I2C write error = ") << r << endl;
    }
  } else {

    for (byte i = 0; i < numbytes; i++) {
      EEPROM[eeaddress + i] = src[i];
    }
  }

  return;
}


//
/// write (or clear) an event to EEPROM
/// just the first four bytes -- NN and EN
//

void CBUSConfig::writeEvent(byte index, byte data[]) {

  int eeaddress = EE_EVENTS_START + (index * EE_BYTES_PER_EVENT);

  // Serial << F("> writeEvent, index = ") << index << F(", addr = ") << eeaddress << endl;
  writeBytesEEPROM(eeaddress, data, 4);

  return;
}

//
/// clear an event from the table
//

void CBUSConfig::cleareventEEPROM(byte index) {

  byte tarray[4] = { 0xff, 0xff, 0xff, 0xff };

  // Serial << F("> clearing event at index = ") << index << endl;
  writeEvent(index, tarray);

  return;
}

//
/// clear all event data in EEPROM
//

void CBUSConfig::resetEEPROM(void) {

  if (eeprom_type == EEPROM_EXTERNAL) {

    Serial << F("> clearing data from external EEPROM ...") << endl;

    for (unsigned int addr = 0; addr < 65535; addr++) {
      writeEEPROM(addr, 0xff);
    }
  }

  return;
}

//
/// reboot via watchdog timer - for AVR core chips
//

#include <avr/wdt.h>

void CBUSConfig::reboot(void) {

  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}

//
/// get free SRAM - for AVR core chips
//

unsigned int CBUSConfig::freeSRAM(void) {

  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

//
/// manually reset the module to factory defaults
//

void CBUSConfig::resetModule(CBUSLED ledGrn, CBUSLED ledYlw, CBUSSwitch pbSwitch) {

    bool bDone;
    unsigned long waittime;

    // start timeout timer
    waittime = millis();
    bDone = false;

    Serial << F("> waiting for a further 5 sec button push, as a safety measure") << endl;

    pbSwitch.reset();
    ledGrn.blink();
    ledYlw.blink();

    // wait for a further (5 sec) button press -- as a 'safety' mechanism
    while (!bDone) {

      // 30 sec timeout
      if ((millis() - waittime) > 30000) {
        Serial << F("> timeout expired, reset not performed") << endl;
        return;
      }

      pbSwitch.run();
      ledGrn.run();
      ledYlw.run();

      // wait until switch held for a further 5 secs
      if (pbSwitch.isPressed() && pbSwitch.getCurrentStateDuration() > 5000) {
        bDone = true;
      }
    }

    // do the reset
    Serial << F("> performing module reset ...") <<  endl;

    ledGrn.off();
    ledYlw.off();
    ledGrn.run();
    ledYlw.run();

    // clear the entire on-chip EEPROM
    Serial << F("> clearing on-chip EEPROM ...") << endl;

    for (unsigned int j = 0; j < EEPROM.length(); j++) {
      EEPROM[j] = 0xff;
    }

    // clear the external I2C EEPROM of learned events
    resetEEPROM();

    // set the node identity defaults
    // we set a NN and CANID of zero in SLiM as we're a consumer-only node
    EEPROM[0] = 0x00;                             // SLiM
    EEPROM[1] = 0x00;                             // CANID
    EEPROM[2] = 0x00;                             // NN hi
    EEPROM[3] = 0x00;                             // NN lo

    FLiM = false;
    nodeNum = 0;
    CANID = nodeNum;

    // zero NVs (NVs number from one, not zero)
    for (byte i = 0; i < EE_NUM_NVS; i++) {
      writeNV(i + 1, 0);
    }

    // reset complete
    reboot();
}

//
/// load NVs from EEPROM
//

void CBUSConfig::loadNVs(void) {

  // load NVs from EEPROM into memory
  FLiM =     EEPROM[0];
  CANID =    EEPROM[1];
  nodeNum = (EEPROM[2] << 8) + EEPROM[3];

  return;
}

//
/// check whether there is a collision for any hash in the event hash table
//

bool CBUSConfig::check_hash_collisions(void) {

byte count = sizeof(evhashtbl) / sizeof(evhashtbl[0]);

  for (byte i = 0; i < count - 1; i++) {
    for (byte j = i + 1; j < count; j++) {
      if (evhashtbl[i] == evhashtbl[j] && evhashtbl[i] != 0) {
        // Serial << F("> hash collision detected, val = ") << evhashtbl[i] << endl;
        // return when first collision detected
        return true;
      }
    }
  }

  return false;
}
