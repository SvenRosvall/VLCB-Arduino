// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

//
/// Controller module configuration
/// manages the storage of events and node variables in on-chip or external EEPROM
//

#include <Streaming.h>

#include "Configuration.h"

#ifdef __AVR__
extern "C" int __heap_start, *__brkval;
#endif

#ifdef __SAM3X8E__
extern "C" char* sbrk(int incr);
#endif

#ifdef ARDUINO_ARCH_RP2040
extern "C" char* sbrk(int incr);
#endif

namespace VLCB
{

//
/// ctor
//

Configuration::Configuration()
{
  this->storage = createDefaultStorageForPlatform();
}

Configuration::Configuration(Storage * theStorage)
{
  this->storage = theStorage;
}

//
/// initialise and set default values
//
void Configuration::begin()
{
  EE_BYTES_PER_EVENT = EE_HASH_BYTES + EE_NUM_EVS;

  storage->begin();
  loadNVs();
  
  if ((currentMode == 0xFF) && (nodeNum == 0xFFFF))   // EEPROM is in factory virgin state
  {
   resetModule();
   clearResetFlag();
   loadNVs();   
  }
  
  makeEvHashTable();  
}

void Configuration::setModuleMode(ModuleMode f)
{
  currentMode = f;
  storage->writeEEPROM(0, f);
}

//
/// store the CANID
//
void Configuration::setCANID(byte canid)
{
  CANID = canid;
  storage->writeEEPROM(1, canid);
}

//
/// store the node number
//
void Configuration::setNodeNum(unsigned int nn)
{
  nodeNum = nn;
  storage->writeEEPROM(2, highByte(nodeNum));
  storage->writeEEPROM(3, lowByte(nodeNum));
}

//
/// lookup an event by node number and event number, using the hash table
//
byte Configuration::findExistingEvent(unsigned int nn, unsigned int en)
{
  byte tarray[EE_HASH_BYTES];
  byte tmphash, i, j, matches;
  bool confirmed = false;

  // DEBUG_SERIAL << F("> looking for match with ") << nn << ", " << en << endl;

  tarray[0] = highByte(nn);
  tarray[1] = lowByte(nn);
  tarray[2] = highByte(en);
  tarray[3] = lowByte(en);

  // calc the hash of the incoming event to match
  tmphash = makeHash(tarray);
  // DEBUG_SERIAL << F("> event hash = ") << tmphash << endl;

  for (i = 0; i < EE_MAX_EVENTS; i++)
  {
    if (evhashtbl[i] == tmphash)
    {
      if (!hash_collision)
      {
        // NN + EN hash matches and there are no hash collisions in the hash table
      }
      else
      {
        // there is a potential hash collision, so we have to check the slower way
        // first, check if this hash appears in the table more than once

        // DEBUG_SERIAL << F("> there are hash collisions") << endl;

        for (j = 0, matches = 0; j < EE_MAX_EVENTS; j++)
        {
          if (evhashtbl[j] == tmphash)
          {
            ++matches;
          }
        }

        if (matches > 1)
        {
          // one or more collisions for this hash exist, so check the very slow way
          // DEBUG_SERIAL << F("> this hash matches = ") << matches << endl;

          for (i = 0; i < EE_MAX_EVENTS; i++)
          {
            if (evhashtbl[i] == tmphash)
            {
              // check the EEPROM for a match with the incoming NN and EN
              readEvent(i, tarray);
              if ((unsigned int)((tarray[0] << 8) + tarray[1]) == nn && (unsigned int)((tarray[2] << 8) + tarray[3]) == en)
              {
                // the stored NN and EN match this event, so no further checking is required
                confirmed = true;
                break;
              }
            }
          }
        }
        else
        {
          // no collisions for this specific hash, so no further checking is required
          break;
        }
      }

      break;
    }
  }

  // finally, there may be a collision with an event that we haven't seen before
  // so we still need to check the candidate match to be certain, if we haven't done so already
  if (i < EE_MAX_EVENTS && !confirmed)
  {
    readEvent(i, tarray);
    if (!((unsigned int)((tarray[0] << 8) + tarray[1]) == nn && (unsigned int)((tarray[2] << 8) + tarray[3]) == en))
    {
      // the stored NN and EN do not match this event
      // DEBUG_SERIAL << F("> hash result does not match stored event") << endl;
      i = EE_MAX_EVENTS;
    }
  }

  // if (i >= EE_MAX_EVENTS) DEBUG_SERIAL << F("> unable to find matching event") << endl;
  return i;
}

//
/// find the first empty EEPROM event slot - the hash table entry == 0
//

byte Configuration::findEventSpace()
{
  byte evidx;

  for (evidx = 0; evidx < EE_MAX_EVENTS; evidx++)
  {
    if (evhashtbl[evidx] == 0)
    {
      // DEBUG_SERIAL << F("> found unused location at index = ") << evidx << endl;
      break;
    }
  }

  return evidx;
}

//
/// create a hash from a 4-byte event entry array -- NN + EN
//
byte Configuration::makeHash(byte tarr[EE_HASH_BYTES])
{
  // make a hash from a 4-byte NN + EN event
  unsigned int nn = (tarr[0] << 8) + tarr[1];
  unsigned int en = (tarr[2] << 8) + tarr[3];

  // need to hash the NN and EN to a uniform distribution across HASH_LENGTH
  byte hash = nn ^ (nn >> 8);
  hash = 7 * hash + (en ^ (en >> 8));

  // ensure it is within bounds and non-zero
  hash %= HASH_LENGTH;
  hash = (hash == 0) ? 255 : hash;

  // DEBUG_SERIAL << F("> makeHash - hash of nn = ") << nn << F(", en = ") << en << F(", = ") << hash << endl;
  return hash;
}

//
/// return an existing EEPROM event as a 4-byte array -- NN + EN
//

void Configuration::readEvent(byte idx, byte tarr[])
{
  // populate the array with the first 4 bytes (NN + EN) of the event entry from the EEPROM
  for (byte i = 0; i < EE_HASH_BYTES; i++)
  {
    tarr[i] = storage->readEEPROM(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + i);
  }

  // DEBUG_SERIAL << F("> readEvent - idx = ") << idx << F(", nn = ") << (tarr[0] << 8) + tarr[1] << F(", en = ") << (tarr[2] << 8) + tarr[3] << endl;
}

//
/// return an event variable (EV) value given the event table index and EV number
//
byte Configuration::getEventEVval(byte idx, byte evnum)
{
  return storage->readEEPROM(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + 3 + evnum);
}

//
/// write an event variable
//
void Configuration::writeEventEV(byte idx, byte evnum, byte evval)
{
  storage->writeEEPROM(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + 3 + evnum, evval);
}

//
/// re/create the event hash table
//
void Configuration::makeEvHashTable()
{
  byte evarray[EE_HASH_BYTES];
  const byte unused_entry[EE_HASH_BYTES] = { 0xff, 0xff, 0xff, 0xff};

  // DEBUG_SERIAL << F("> creating event hash table") << endl;

  evhashtbl = (byte *)malloc(EE_MAX_EVENTS * sizeof(byte));

  for (byte idx = 0; idx < EE_MAX_EVENTS; idx++)
  {
    readEvent(idx, evarray);

    // empty slots have all four bytes set to 0xff
    if (memcmp(evarray, unused_entry, EE_HASH_BYTES) == 0)
    {
      evhashtbl[idx] = 0;
    }
    else
    {
      evhashtbl[idx] = makeHash(evarray);
    }
  }

  hash_collision = check_hash_collisions();
}

//
/// update a single hash table entry -- after a learn or unlearn
//
void Configuration::updateEvHashEntry(byte idx)
{
  byte evarray[EE_HASH_BYTES];
  const byte unused_entry[EE_HASH_BYTES] = { 0xff, 0xff, 0xff, 0xff};

  // read the first four bytes from EEPROM - NN + EN
  readEvent(idx, evarray);

  // empty slots have all four bytes set to 0xff
  if (memcmp(evarray, unused_entry, EE_HASH_BYTES) == 0)
  {
    evhashtbl[idx] = 0;
  }
  else
  {
    evhashtbl[idx] = makeHash(evarray);
  }

  hash_collision = check_hash_collisions();

  // DEBUG_SERIAL << F("> updateEvHashEntry for idx = ") << idx << F(", hash = ") << hash << endl;
}

//
/// clear the hash table
//
void Configuration::clearEvHashTable()
{
  // zero in the hash table indicates that the corresponding event slot is free
  // DEBUG_SERIAL << F("> clearEvHashTable - clearing hash table") << endl;

  for (byte i = 0; i < EE_MAX_EVENTS; i++)
  {
    evhashtbl[i] = 0;
  }

  hash_collision = false;
}

//
/// print the event hash table
//
void Configuration::printEvHashTable(bool raw)
{
  // removed so that no libraries produce serial output
  // can be implemented in user's sketch
  (void)raw;

  /*
    DEBUG_SERIAL << F("> Event hash table - ") << hash_collision << endl;

    for (byte i = 0; i < EE_MAX_EVENTS; i++) {
      if (evhashtbl[i] > 0) {
        if (raw)
          DEBUG_SERIAL << evhashtbl[i] << endl;
        else
          DEBUG_SERIAL << i << " - " << evhashtbl[i] << endl;
      }
    }

    DEBUG_SERIAL << endl;
  */
}

//
/// return the number of stored events
//
byte Configuration::numEvents()
{
  byte numevents = 0;

  for (byte i = 0; i < EE_MAX_EVENTS; i++)
  {
    if (evhashtbl[i] != 0)
    {
      ++numevents;
    }
  }

  return numevents;
}

//
/// return a single hash table entry by index
//
byte Configuration::getEvTableEntry(byte tindex)
{
  if (tindex < EE_MAX_EVENTS)
  {
    return evhashtbl[tindex];
  }
  else
  {
    return 0;
  }
}

//
/// read an NV value from EEPROM
/// note that NVs number from 1, not 0
//
byte Configuration::readNV(byte idx)
{
  return (storage->readEEPROM(EE_NVS_START + (idx - 1)));
}

//
/// write an NV value to EEPROM
/// note that NVs number from 1, not 0
//
void Configuration::writeNV(byte idx, byte val)
{
  storage->writeEEPROM(EE_NVS_START + (idx - 1), val);
}

//
/// generic EEPROM access methods
//

//
/// write (or clear) an event to EEPROM
/// just the first four bytes -- NN and EN
//
void Configuration::writeEvent(byte index, byte data[EE_HASH_BYTES])
{
  unsigned int eeaddress = EE_EVENTS_START + (index * EE_BYTES_PER_EVENT);

  // DEBUG_SERIAL << F("> writeEvent, index = ") << index << F(", addr = ") << eeaddress << endl;
  storage->writeBytesEEPROM(eeaddress, data, EE_HASH_BYTES);
}

//
/// clear an event from the table
//
void Configuration::cleareventEEPROM(byte index)
{
  byte unused_entry[4] = { 0xff, 0xff, 0xff, 0xff };

  // DEBUG_SERIAL << F("> clearing event at index = ") << index << endl;
  writeEvent(index, unused_entry);
}

//
/// reboot the processor
//

#ifdef __AVR__
#include <avr/wdt.h>
#endif

#ifndef AVR_RESET_METHOD
#define AVR_RESET_METHOD 4     // don't use watchdog timer method as it's unreliable on some boards
#endif

void (*rebootFunc)() = 0;  // just causes a jump to address zero - not a full chip reset

void Configuration::reboot()
{
#ifdef __AVR__

// for newer AVR Xmega, e.g. AVR-DA
// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
  // CCP = (uint8_t)CCP_IOREG_gc;
  // RSTCTRL.SWRR = 1;
#else

// for older AVR Mega, e.g. Uno, Nano, Mega
#if AVR_RESET_METHOD == 1      // 1. the preferred way
// #warning "Using reset method 1"
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
#endif

#if AVR_RESET_METHOD == 2      // 2. a variation on the watchdog method
// #warning "Using reset method 2"
  wdt_enable(WDTO_15MS);
  while (1) {}
#endif

#if AVR_RESET_METHOD == 3      // 3. a dirty way
// #warning "Using reset method 3"
  asm volatile ("jmp 0");
#endif

#if AVR_RESET_METHOD == 4      // 4. another dirty way
// #warning "Using reset method 4"
  rebootFunc();
#endif

#endif  // which AVR
#endif  // AVR

// for ESP32
#if defined ESP32 || defined ESP8266
  ESP.restart();
#endif

// for Arduino Due
#ifdef __SAM3X8E__
  rstc_start_software_reset(RSTC);
#endif

// for Raspberry Pi Pico using arduino-pico core
#ifdef ARDUINO_ARCH_RP2040
  watchdog_enable(100, 1);      // set watchdog timeout to 100ms and allow to expire
  while (1);
#endif
}

//
/// get free RAM
//
unsigned int Configuration::freeSRAM()
{
#ifdef __AVR__
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
#endif

#if defined ESP32 || defined ESP8266
  return ESP.getFreeHeap();
#endif

#ifdef __SAM3X8E__
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#endif

#ifdef ARDUINO_ARCH_RP2040
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#endif

  // For other platforms you will get a warning about missing return.
}

//
/// manually reset the module to factory defaults
//
void Configuration::resetModule(UserInterface * ui)
{
  /// standard implementation of resetModule()

  bool bDone;
  unsigned long waittime;

  // start timeout timer
  waittime = millis();
  bDone = false;

  // DEBUG_SERIAL << F("> waiting for a further 5 sec button push, as a safety measure") << endl;

  ui->indicateResetting();

  // wait for a further (5 sec) button press -- as a 'safety' mechanism
  while (!bDone)
  {
    // 30 sec timeout
    if ((millis() - waittime) > 30000)
    {
      // DEBUG_SERIAL << F("> timeout expired, reset not performed") << endl;
      return;
    }

    ui->run();

    // wait until switch held for a further 5 secs
    if (ui->resetRequested())
    {
      bDone = true;
    }
  }

  // do the reset
  // DEBUG_SERIAL << F("> performing module reset ...") <<  endl;
  ui->indicateResetDone();
  resetModule();
}

void Configuration::resetModule()
{
  /// implementation of resetModule() without VLCB Switch or LEDs
  // uint32_t t = millis();
  // DEBUG_SERIAL << F("> resetting EEPROM") << endl;

  // clear the learned events from storage
  storage->resetEEPROM();

  // DEBUG_SERIAL << F("> setting Uninitialised config") << endl;

  // set the node identity defaults
  // we set a NN and CANID of zero in Uninitialised as we're now a consumer-only node

  storage->writeEEPROM(0, 0);     // Mode = Uninitialised
  storage->writeEEPROM(1, 0);     // CANID
  storage->writeEEPROM(2, 0);     // NN hi
  storage->writeEEPROM(3, 0);     // NN lo
  setResetFlag();        // set reset indicator

  // zero NVs (NVs number from one, not zero)
  for (byte i = 0; i < EE_NUM_NVS; i++)
  {
    writeNV(i + 1, 0);
  }

  // DEBUG_SERIAL << F("> complete in ") << (millis() - t) << F(", rebooting ... ") << endl;

  // reset complete
  reboot();
}

//
/// load node identity from EEPROM
//
void Configuration::loadNVs()
{
  currentMode = (ModuleMode) storage->readEEPROM(0);
  CANID =    storage->readEEPROM(1);
  nodeNum =  (storage->readEEPROM(2) << 8) + storage->readEEPROM(3);
}

//
/// check whether there is a collision for any hash in the event hash table
//
bool Configuration::check_hash_collisions()
{
  for (byte i = 0; i < EE_MAX_EVENTS - 1; i++)
  {
    for (byte j = i + 1; j < EE_MAX_EVENTS; j++)
    {
      if (evhashtbl[i] == evhashtbl[j] && evhashtbl[i] != 0)
      {
        // DEBUG_SERIAL << F("> hash collision detected, val = ") << evhashtbl[i] << endl;
        // return when first collision detected
        return true;
      }
    }
  }

  return false;
}

//
/// a group of methods to get and set the reset flag
/// the resetModule method writes the value 99 to EEPROM address 5 when a module reset has been performed
/// this can be tested at module startup for e.g. setting default NVs or creating producer events
//
void Configuration::setResetFlag()
{
  storage->writeEEPROM(5, 99);
}

void Configuration::clearResetFlag()
{
  storage->writeEEPROM(5, 0);
}

bool Configuration::isResetFlagSet()
{
  return (storage->readEEPROM(5) == 99);
}

}