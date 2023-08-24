#include <Arduino.h>
#include "TestTools.hpp"

namespace
{

void testBits()
{
  test();

  assertEquals(1, bitRead(0x33, 4));
  assertEquals(0, bitRead(0x33, 3));
}

}

void testArduino()
{
  testBits();
}
