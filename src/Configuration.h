// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>                // for definition of byte datatype

#include <UserInterface.h>
#include "Storage.h"

namespace VLCB
{

// in-memory hash table
static const byte EE_HASH_BYTES = 4;
static const byte HASH_LENGTH = 128;

//
/// Controller modes
//

enum ModuleMode {
  MODE_UNINITIALISED = 0,
  MODE_NORMAL = 1,
  MODE_SETUP = 2,
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

  void printEvHashTable(bool raw);
  byte getEvTableEntry(byte tindex);
  byte numEvents();
  void updateEvHashEntry(byte idx);
  void clearEvHashTable();
  byte getEventEVval(byte idx, byte evnum);
  void writeEventEV(byte idx, byte evnum, byte evval);

  byte readNV(byte idx);
  void writeNV(byte idx, byte val);

  void readEvent(byte idx, byte tarr[]);
  void writeEvent(byte index, byte data[]);
  void cleareventEEPROM(byte index);
  void resetModule(UserInterface * ui);
  void resetModule();

  void setCANID(byte canid);
  void setModuleMode(ModuleMode m);
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
  ModuleMode currentMode;
  unsigned int nodeNum;

  // These functions shouldn't be here. But keep them for now.
  unsigned int freeSRAM();
  void reboot();

private:
  Storage * storage;

  // Stuff below are not confirmed to be needed to be publically available.
private:
  byte makeHash(byte tarr[]);
  void getEvArray(byte idx);
  void makeEvHashTable();
  bool check_hash_collisions();

  void loadNVs();

  unsigned int getEVAddress(byte idx, byte evnum);

  byte *evhashtbl;
  bool hash_collision;
};

}