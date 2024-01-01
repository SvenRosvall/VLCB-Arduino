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
  LOCATION_RESET_FLAG = 5
};

enum FlagBits {
  HEARTBEAT_BIT = 0,
  EVENT_ACK_BIT = 1,
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

  byte findExistingEvent(unsigned int nn, unsigned int en);
  byte findEventSpace();
  byte findExistingEventByEv(byte evindex, byte evval);

  void printEvHashTable(bool raw);
  byte getEvTableEntry(byte tindex);
  byte numEvents();
  void updateEvHashEntry(byte idx);
  void clearEvHashTable();
  byte getEventEVval(byte idx, byte evnum);
  void writeEventEV(byte idx, byte evnum, byte evval);

  byte readNV(byte idx);
  void writeNV(byte idx, byte val);

  void readEvent(byte idx, byte tarr[EE_HASH_BYTES]);
  void writeEvent(byte index, const byte data[EE_HASH_BYTES]);
  void cleareventEEPROM(byte index);
  void resetModule();

  void setCANID(byte canid);
  void setModuleMode(VlcbModeParams m);
  void setHeartbeat(bool beat);
  void setNodeNum(unsigned int nn);
  void setEventAck(bool ea);

  void setResetFlag();
  void clearResetFlag();
  bool isResetFlagSet();

  unsigned int EE_EVENTS_START;
  byte EE_MAX_EVENTS;
  byte EE_NUM_EVS;
  byte EE_BYTES_PER_EVENT;
  unsigned int EE_NVS_START;
  byte EE_NUM_NVS;
  byte EE_PRODUCED_EVENTS;

  bool heartbeat;
  bool eventAck;
  byte CANID;
  VlcbModeParams currentMode;
  unsigned int nodeNum;

  // These functions shouldn't be here. But keep them for now.
  unsigned int freeSRAM();
  void reboot();

private:
  Storage * storage;

  // Stuff below are not confirmed to be needed to be publically available.
private:
  byte makeHash(byte tarr[EE_HASH_BYTES]);
  void getEvArray(byte idx);
  void makeEvHashTable();

  void loadNVs();

  unsigned int getEVAddress(byte idx, byte evnum);

  byte *evhashtbl;
};

}
