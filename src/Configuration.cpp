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

static const byte unused_entry[EE_HASH_BYTES] = { 0xff, 0xff, 0xff, 0xff};

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
  if (EE_EVENTS_START == 0)
  {
    EE_EVENTS_START = EE_NVS_START + EE_NUM_NVS;
    // Note: The formula above does not allow for upgrades to user app where NVs are added 
    // as this would move the location for stored events. 
  }

  storage->begin();
  loadNVs();

  if ((storage->read(LOCATION_MODE) == 0xFF) && (nodeNum == 0xFFFF))   // EEPROM is in factory virgin state
  {
    // DEBUG_SERIAL << "Configuration::begin() - EEPROM is factory reset. Resetting module." << endl;
    resetModule();
    clearResetFlag();
    loadNVs();
  }

  makeEvHashTable();
}

void Configuration::setModuleUninitializedMode()
{
  setModuleMode(MODE_UNINITIALISED);
  setNodeNum(0);
}

void Configuration::setModuleNormalMode(unsigned int nodeNumber)
{
  setModuleMode(MODE_NORMAL);
  setNodeNum(nodeNumber);
}

void Configuration::setModuleMode(VlcbModeParams f)
{
  currentMode = f;
  storage->write(LOCATION_MODE, f);
}

void Configuration::setHeartbeat(bool beat)
{
  heartbeat = beat;
  byte mode = storage->read(LOCATION_FLAGS);
  bitWrite(mode, HEARTBEAT_BIT, beat);
  storage->write(LOCATION_FLAGS, mode);
}

void Configuration::setEventAck(bool ea)
{
  eventAck = ea;
  byte servicePersist = storage->read(LOCATION_FLAGS);
  bitWrite(servicePersist, HEARTBEAT_BIT, ea);
  storage->write(LOCATION_FLAGS, servicePersist);
}

//
/// store the CANID
//
void Configuration::setCANID(byte canid)
{
  CANID = canid;
  storage->write(LOCATION_CANID, canid);
}

//
/// store the node number
//
void Configuration::setNodeNum(unsigned int nn)
{
  nodeNum = nn;
  storage->write(LOCATION_NODE_NUMBER_HIGH, highByte(nodeNum));
  storage->write(LOCATION_NODE_NUMBER_LOW, lowByte(nodeNum));
}

//
/// lookup an event by node number and event number, using the hash table
//
byte Configuration::findExistingEvent(unsigned int nn, unsigned int en)
{
  byte tarray[EE_HASH_BYTES];

  // DEBUG_SERIAL << F("> looking for match with ") << nn << ", " << en << endl;

  setTwoBytes(&tarray[0], nn);
  setTwoBytes(&tarray[2], en);

  // calc the hash of the incoming event to match
  byte tmphash = makeHash(tarray);
  // DEBUG_SERIAL << F("> event hash = ") << tmphash << endl;

  for (byte i = 0; i < EE_MAX_EVENTS; i++)
  {
    if (evhashtbl[i] == tmphash)
    {
      // check the EEPROM for a match with the incoming NN and EN
      readEvent(i, tarray);
      if (getTwoBytes(&tarray[0]) == nn && getTwoBytes(&tarray[2]) == en)
      {
        return i;
      }
    }
  }

  // DEBUG_SERIAL << F("> unable to find matching event") << endl;
  return EE_MAX_EVENTS;
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

byte Configuration::findExistingEventByEv(byte evnum, byte evval)
{
  byte i;
  for (i = 0; i < EE_MAX_EVENTS; i++)
  {
    if (getEventEVval(i, evnum) == evval)
    {
      break;
    }
  }
  return i;
}

//
/// create a hash from a 4-byte event entry array -- NN + EN
//
byte Configuration::makeHash(byte tarr[EE_HASH_BYTES])
{
  // make a hash from a 4-byte NN + EN event
  unsigned int nn = getTwoBytes(&tarr[0]);
  unsigned int en = getTwoBytes(&tarr[2]);

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

void Configuration::readEvent(byte idx, byte tarr[EE_HASH_BYTES])
{
  // populate the array with the first 4 bytes (NN + EN) of the event entry from the EEPROM
  for (byte i = 0; i < EE_HASH_BYTES; i++)
  {
    tarr[i] = storage->read(EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + i);
  }

  // DEBUG_SERIAL << F("> readEvent - idx = ") << idx << F(", nn = ") << getTwoBytes(&tarr[0]) << F(", en = ") << getTwoBytes(&tarr[2]) << endl;
}

// return the address an event variable is stored in the eeprom.
// Note that the evnum is 1 based and needs to be converted to 0 based.
unsigned int Configuration::getEVAddress(byte idx, byte evnum)
{
  return EE_EVENTS_START + (idx * EE_BYTES_PER_EVENT) + EE_HASH_BYTES + evnum - 1;
}

//
/// return an event variable (EV) value given the event table index and EV number
//
byte Configuration::getEventEVval(byte idx, byte evnum)
{
  return storage->read(getEVAddress(idx, evnum));
}

//
/// write an event variable
//
void Configuration::writeEventEV(byte idx, byte evnum, byte evval)
{
  storage->write(getEVAddress(idx, evnum), evval);
}

//
/// re/create the event hash table
//
void Configuration::makeEvHashTable()
{
  byte evarray[EE_HASH_BYTES];

  // DEBUG_SERIAL << F("> creating event hash table") << endl;

  evhashtbl = (byte *)malloc(EE_MAX_EVENTS * sizeof(byte));

  for (byte idx = 0; idx < EE_MAX_EVENTS; idx++)
  {
    updateEvHashEntry(idx);
  }
}

//
/// update a single hash table entry -- after a learn or unlearn
//
void Configuration::updateEvHashEntry(byte idx)
{
  byte evarray[EE_HASH_BYTES];

  // read the first four bytes from EEPROM - NN + EN
  readEvent(idx, evarray);

  // empty slots have all four bytes set to 0xff
  if (nnenEquals(evarray, unused_entry))
  {
    evhashtbl[idx] = 0;
  }
  else
  {
    evhashtbl[idx] = makeHash(evarray);
  }

  // DEBUG_SERIAL << F("> updateEvHashEntry for idx = ") << idx << F(", hash = ") << hash << endl;
}

// Return a readable string for a mode value
const char * Configuration::modeString(VlcbModeParams mode)
{
  switch (mode & 0x07)
  {
    case MODE_NORMAL: return "Normal";
    case MODE_UNINITIALISED: return "Uninitialised";
    case MODE_SETUP: return "Setup";
    default: return "Unknown"; 
  }
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
    DEBUG_SERIAL << F("> Event hash table - ") << endl;

    for (byte i = 0; i < EE_MAX_EVENTS; i++) 
    {
      if (evhashtbl[i] > 0)
      {
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
  return (storage->read(EE_NVS_START + (idx - 1)));
}

//
/// write an NV value to EEPROM
/// note that NVs number from 1, not 0
//
void Configuration::writeNV(byte idx, byte val)
{
  storage->write(EE_NVS_START + (idx - 1), val);
}

//
/// generic EEPROM access methods
//

//
/// write (or clear) an event to EEPROM
/// just the first four bytes -- NN and EN
//
void Configuration::writeEvent(byte index, const byte data[EE_HASH_BYTES])
{
  unsigned int eeaddress = EE_EVENTS_START + (index * EE_BYTES_PER_EVENT);

  // DEBUG_SERIAL << F("> writeEvent, index = ") << index << F(", addr = ") << eeaddress << endl;
  storage->writeBytes(eeaddress, data, EE_HASH_BYTES);
}

//
/// clear an event from the table
//
void Configuration::cleareventEEPROM(byte index)
{
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
#endif  // __AVR__

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

#elif defined ESP32 || defined ESP8266
  return ESP.getFreeHeap();

#elif defined __SAM3X8E__
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));

#elif defined ARDUINO_ARCH_RP2040
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));

#else
  return 0;

#endif

  // For other platforms you will get a warning about missing return.
}

//
/// reset the module to factory defaults
//
void Configuration::resetModule()
{
  /// implementation of resetModule() without VLCB Switch or LEDs
  // uint32_t t = millis();
  // DEBUG_SERIAL << F("> resetting EEPROM") << endl;

  // clear the learned events from storage
  storage->reset();

  // DEBUG_SERIAL << F("> setting Uninitialised config") << endl;

  // set the node identity defaults
  // we set a NN and CANID of zero and a mode Uninitialised

  storage->write(LOCATION_MODE, MODE_UNINITIALISED);
  storage->write(LOCATION_CANID, 0);
  storage->write(LOCATION_NODE_NUMBER_HIGH, 0);
  storage->write(LOCATION_NODE_NUMBER_LOW, 0);
  storage->write(LOCATION_FLAGS, 0);
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
  currentMode = (VlcbModeParams) (storage->read(LOCATION_MODE) ); // Bit 0 persists Uninitialised / Normal mode
  if (currentMode == VlcbModeParams::MODE_SETUP)
  {
    // currentMode should never be setup but may happen on re-initialized boards.
    currentMode = VlcbModeParams::MODE_UNINITIALISED;
    storage->write(LOCATION_MODE, currentMode);
  }
  CANID = storage->read(LOCATION_CANID);
  nodeNum = (storage->read(LOCATION_NODE_NUMBER_HIGH) << 8) + storage->read(LOCATION_NODE_NUMBER_LOW);
  byte flags = storage->read(LOCATION_FLAGS);
  heartbeat = flags & (1 << HEARTBEAT_BIT);
  eventAck = flags & (1 << EVENT_ACK_BIT);
  // DEBUG_SERIAL << "Configuration::loadNVs() mode=" << currentMode << ", CANID=" << CANID << ", nodeNum=" << nodeNum << ", flags=" << _HEX(flags) << endl;
}

//
/// a group of methods to get and set the reset flag
/// the resetModule method writes the value 99 to EEPROM address 5 when a module reset has been performed
/// this can be tested at module startup for e.g. setting default NVs or creating producer events
//
void Configuration::setResetFlag()
{
  storage->write(LOCATION_RESET_FLAG, 99);
}

void Configuration::clearResetFlag()
{
  storage->write(LOCATION_RESET_FLAG, 0);
}

bool Configuration::isResetFlagSet()
{
  return (storage->read(LOCATION_RESET_FLAG) == 99);
}

void Configuration::setTwoBytes(byte *target, unsigned int value)
{
  target[0] = highByte(value);
  target[1] = lowByte(value);
}

unsigned int Configuration::getTwoBytes(const byte *bytes)
{
  return (bytes[0] << 8) + bytes[1];
}

bool Configuration::nnenEquals(const byte lhs[EE_HASH_BYTES], const byte rhs[EE_HASH_BYTES])
{
  return memcmp(rhs, lhs, EE_HASH_BYTES) == 0;
}

}
