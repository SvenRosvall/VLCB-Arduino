// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>                // for definition of byte datatype

#include "Storage.h"
#include "vlcbdefs.hpp"

namespace VLCB
{

// in-memory hash table
static const byte EE_HASH_BYTES = 4;
static const byte HASH_LENGTH = 128;

enum EepromLocations {
  LOCATION_MODE = 0,
  LOCATION_CANID = 1,
  LOCATION_NODE_NUMBER_HIGH = 2,
  LOCATION_NODE_NUMBER_LOW = 3,
  LOCATION_FLAGS = 4,
  LOCATION_RESET_FLAG = 5,
  LOCATION_RESERVED_SIZE = 10 // NVs/EVs can start from here.
};

enum FlagBits {
  HEARTBEAT_BIT = 0,
  EVENT_ACK_BIT = 1,
  FCU_COMPATIBLE_BIT = 2
};

//
/// a class to encapsulate Controller module configuration, events, NVs, EEPROM, etc
//

class Configuration
{
public:
  Configuration();
  Configuration(Storage * theStorage);
  void begin();

  byte findExistingEvent(unsigned int nn, unsigned int en) const;
  byte findEventSpace() const;
  byte findExistingEventByEv(byte evnum, byte evval) const;
  
  void printEvHashTable(bool raw);
  byte getEvTableEntry(byte tindex) const;
  byte numEvents() const;
  void updateEvHashEntry(byte idx);
  void clearEvHashTable();
  byte getEventEVval(byte idx, byte evnum) const;
  void writeEventEV(byte idx, byte evnum, byte evval);

  byte readNV(byte idx) const;
  void writeNV(byte idx, byte val);

  void readEvent(byte idx, byte tarr[EE_HASH_BYTES]) const;
  void writeEvent(byte index, const byte data[EE_HASH_BYTES]);
  void cleareventEEPROM(byte index);
  void resetModule();
  void commitToEEPROM();

  void setCANID(byte canid);
  void setModuleUninitializedMode();
  void setModuleNormalMode(unsigned int nodeNumber);
  void setHeartbeat(bool beat);
  void setNodeNum(unsigned int nn);
  void setEventAck(bool ea);
  void setFcuCompatability(bool fcu);

  void setResetFlag();
  void clearResetFlag();
  bool isResetFlagSet();

  unsigned int EE_NVS_START = LOCATION_RESERVED_SIZE;
  byte EE_NUM_NVS = 0;
  unsigned int EE_EVENTS_START = 0; // Value calculated in begin() unless set by user.
  byte EE_MAX_EVENTS = 0;
  byte EE_NUM_EVS = 0;
  byte EE_PRODUCED_EVENTS = 0;
  byte EE_BYTES_PER_EVENT; // Value calculated in begin()
  unsigned int EE_FREE_BASE; // Value calculated in begin()
  unsigned int EE_USER_BYTES = 0; // Specified by user in setup for ESP processors.

  bool heartbeat;
  bool eventAck;
  bool fcuCompatible;
  byte CANID;
  VlcbModeParams currentMode;
  unsigned int nodeNum;

  // These functions shouldn't be here. But keep them for now.
  unsigned int freeSRAM();
  void reboot();

  static void setTwoBytes(byte *target, unsigned int value);
  static unsigned int getTwoBytes(const byte *bytes);
  static bool nnenEquals(const byte lhs[EE_HASH_BYTES], const byte rhs[EE_HASH_BYTES]);
  static const char * modeString(VlcbModeParams mode);

private:
  Storage * storage;

  void setModuleMode(VlcbModeParams m);
  byte makeHash(byte tarr[EE_HASH_BYTES]) const;
  void makeEvHashTable();

  void loadNVs();

  unsigned int getEVAddress(byte idx, byte evnum) const;

  byte *evhashtbl;
};

}
