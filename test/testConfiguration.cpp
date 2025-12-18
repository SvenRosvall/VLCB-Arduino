//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Limited testing of Configuration class.
// Only created this for refactoring of Configuration::findExistingEvent()

#include "Configuration.h"
#include "TestTools.hpp"
#include "VlcbCommon.h"
#include "MockStorage.h"

namespace
{
const int NOTFOUND = 20;

void testDefaultEepromValues()
{
  test();

  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  VLCB::Configuration * configuration = createConfiguration(mockStorage.get());
  configuration->begin();

  assertEquals(VLCB::LOCATION_RESERVED_SIZE, configuration->EE_NVS_START);
  assertEquals(0, configuration->getNumNodeVariables());
  assertEquals(VLCB::LOCATION_RESERVED_SIZE, configuration->EE_EVENTS_START);
  assertEquals(0, configuration->getNumEvents());
  assertEquals(0, configuration->getNumEVs());
  assertEquals(VLCB::EE_HASH_BYTES, configuration->EE_BYTES_PER_EVENT);
}

void testCalculatedEepromValues()
{
  test();

  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  VLCB::Configuration * configuration = createConfiguration(mockStorage.get());
  configuration->setNumNodeVariables(3);
  configuration->setNumEvents(7);
  configuration->setNumEVs(2);
  configuration->begin();

  assertEquals(VLCB::LOCATION_RESERVED_SIZE, configuration->EE_NVS_START);
  assertEquals(3, configuration->getNumNodeVariables());
  assertEquals(VLCB::LOCATION_RESERVED_SIZE + 3, configuration->EE_EVENTS_START);
  assertEquals(7, configuration->getNumEvents());
  assertEquals(2, configuration->getNumEVs());
  assertEquals(VLCB::EE_HASH_BYTES + 2, configuration->EE_BYTES_PER_EVENT);
}

void testLoadNVsFromZeroedEEPROM()
{
  // Sometimes when a module is reprogrammed the EEPROM gets zero-ed rather than getting all 0xFF.
  // This upsets currentMode.
  test();
  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  for (unsigned int addr = 0; addr < 100; ++addr)
  {
    mockStorage->write(addr, 0);
  }
  VLCB::Configuration *configuration = createConfiguration(mockStorage.get());
  configuration->begin();

  assertEquals(VlcbModeParams::MODE_UNINITIALISED, configuration->currentMode);
}

void testIsEventSlotInUse()
{
  test();
  
  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 8);
  configuration->updateEvHashEntry(3);
  
  assertEquals(false, configuration->isEventSlotInUse(2));
  assertEquals(true, configuration->isEventSlotInUse(3));
}

void testFindEventInEmptyTable()
{
  test();
  
  VLCB::Configuration * configuration = createConfiguration();

  int result = configuration->findExistingEvent(12, 34);

  assertEquals(NOTFOUND, result);
}

void testFindEventFound()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 8);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 8);

  assertEquals(3, result);
}

void testFindEventNotFound()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 8);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(NOTFOUND, result);
}

void testFindEventNotFoundWithSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  // Use an nn/en combination that yields the same hash as the one below.
  configuration->writeEvent(3, 6, 264);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(NOTFOUND, result);
}

void testFindEventFoundWithSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 264);
  configuration->updateEvHashEntry(3);

  configuration->writeEvent(5, 6, 9);
  configuration->updateEvHashEntry(5);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(5, result);
}

void testFindEventFoundWithOtherSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 264);
  configuration->updateEvHashEntry(3);

  configuration->writeEvent(5, 6, 9);
  configuration->updateEvHashEntry(5);

  configuration->writeEvent(7, 6, 11);
  configuration->updateEvHashEntry(7);

  int result = configuration->findExistingEvent(6, 11);

  assertEquals(7, result);
}

void testFindEventNotFoundWithOtherSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 264);
  configuration->updateEvHashEntry(3);

  configuration->writeEvent(5, 6, 9);
  configuration->updateEvHashEntry(5);

  int result = configuration->findExistingEvent(6, 11);

  assertEquals(NOTFOUND, result);
}

void testFindEventMultiple()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(3, 6, 9);
  configuration->updateEvHashEntry(3);

  configuration->writeEvent(4, 6, 8);
  configuration->updateEvHashEntry(4);

  configuration->writeEvent(5, 6, 9);
  configuration->updateEvHashEntry(5);

  int result = configuration->findExistingEvent(6, 9);
  assertEquals(3, result);

  result = configuration->findExistingEvent(6, 9, result + 1);
  assertEquals(5, result);
}

void testFindEventByEv()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  configuration->writeEvent(2, 6, 7);
  configuration->writeEventEV(2, 1, 41);
  configuration->updateEvHashEntry(2);

  configuration->writeEvent(3, 6, 8);
  configuration->writeEventEV(3, 1, 42);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEventByEv(1, 42);

  assertEquals(3, result);
}

}

void testConfiguration()
{
  testDefaultEepromValues();
  testCalculatedEepromValues();
  testLoadNVsFromZeroedEEPROM();
  testIsEventSlotInUse();
  testFindEventInEmptyTable();
  testFindEventFound();
  testFindEventNotFound();
  testFindEventNotFoundWithSameHash();
  testFindEventFoundWithSameHash();
  testFindEventFoundWithOtherSameHash();
  testFindEventNotFoundWithOtherSameHash();
  testFindEventMultiple();
  testFindEventByEv();
}