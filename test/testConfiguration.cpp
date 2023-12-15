//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Limited testing of Configuration class.
// Only created this for refactoring of Configuration::findExistingEvent()

#include "Configuration.h"
#include "TestTools.hpp"
#include "VlcbCommon.h"

namespace
{
const int NOTFOUND = 20;

void testFindInEmptyTable()
{
  test();
  
  VLCB::Configuration * configuration = createConfiguration();

  int result = configuration->findExistingEvent(12, 34);

  assertEquals(NOTFOUND, result);
}

void testFindFound()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData[VLCB::EE_HASH_BYTES] = {0, 6, 0, 0x08};
  configuration->writeEvent(3, eventData);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 8);

  assertEquals(3, result);
}

void testFindNotFound()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData[VLCB::EE_HASH_BYTES] = {0, 6, 0, 0x08};
  configuration->writeEvent(3, eventData);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(NOTFOUND, result);
}

void testFindNotFoundWithSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData[VLCB::EE_HASH_BYTES] = {0, 6, 0x01, 0x08};
  configuration->writeEvent(3, eventData);
  configuration->updateEvHashEntry(3);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(NOTFOUND, result);
}

void testFindFoundWithSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData1[VLCB::EE_HASH_BYTES] = {0, 6, 0x01, 0x08};
  configuration->writeEvent(3, eventData1);
  configuration->updateEvHashEntry(3);

  byte eventData2[VLCB::EE_HASH_BYTES] = {0, 6, 0x00, 0x09};
  configuration->writeEvent(5, eventData2);
  configuration->updateEvHashEntry(5);

  int result = configuration->findExistingEvent(6, 9);

  assertEquals(5, result);
}

void testFindFoundWithOtherSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData1[VLCB::EE_HASH_BYTES] = {0, 6, 0x01, 0x08};
  configuration->writeEvent(3, eventData1);
  configuration->updateEvHashEntry(3);

  byte eventData2[VLCB::EE_HASH_BYTES] = {0, 6, 0, 9};
  configuration->writeEvent(5, eventData2);
  configuration->updateEvHashEntry(5);

  byte eventData3[VLCB::EE_HASH_BYTES] = {0, 6, 0, 11};
  configuration->writeEvent(7, eventData3);
  configuration->updateEvHashEntry(7);

  int result = configuration->findExistingEvent(6, 11);

  assertEquals(7, result);
}

void testFindNotFoundWithOtherSameHash()
{
  test();

  VLCB::Configuration * configuration = createConfiguration();

  byte eventData1[VLCB::EE_HASH_BYTES] = {0, 6, 0x01, 0x08};
  configuration->writeEvent(3, eventData1);
  configuration->updateEvHashEntry(3);

  byte eventData2[VLCB::EE_HASH_BYTES] = {0, 6, 0x00, 0x09};
  configuration->writeEvent(5, eventData2);
  configuration->updateEvHashEntry(5);

  int result = configuration->findExistingEvent(6, 11);

  assertEquals(NOTFOUND, result);
}

}

void testConfiguration()
{
  testFindInEmptyTable();
  testFindFound();
  testFindNotFound();
  testFindNotFoundWithSameHash();
  testFindFoundWithSameHash();
  testFindFoundWithOtherSameHash();
  testFindNotFoundWithOtherSameHash();
}