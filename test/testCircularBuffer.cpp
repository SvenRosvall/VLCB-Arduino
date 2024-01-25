//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "TestTools.hpp"
#include "CircularBuffer.h"

namespace
{

void testEmpty()
{
  test();
  
  VLCB::CircularBuffer<int> buffer;

  assertEquals(false, buffer.available());
  assertEquals(0, buffer.getHighWaterMark());
  assertEquals(0, buffer.getOverflows());
  assertEquals(0, buffer.getNumberOfGets());
  assertEquals(0, buffer.getNumberOfPuts());
}

void testHasOne()
{
  test();
  
  VLCB::CircularBuffer<int> buffer(4);
  int entry = 17;
  buffer.put(entry);

  assertEquals(true, buffer.available());
  assertEquals(1, buffer.getHighWaterMark());
  assertEquals(0, buffer.getOverflows());
  assertEquals(0, buffer.getNumberOfGets());
  assertEquals(1, buffer.getNumberOfPuts());

  assertEquals(entry, buffer.pop());
  assertEquals(1, buffer.getNumberOfGets());
  assertEquals(false, buffer.available());
}

void testFull()
{
  test();
  
  VLCB::CircularBuffer<int> buffer(4);
  int entry = 1;
  buffer.put(entry);
  buffer.put(entry);
  buffer.put(entry);
  buffer.put(entry);

  assertEquals(true, buffer.available());
  assertEquals(4, buffer.getHighWaterMark());
  assertEquals(0, buffer.getOverflows());
  assertEquals(0, buffer.getNumberOfGets());
  assertEquals(4, buffer.getNumberOfPuts());
}

void testOverflow()
{
  test();
  
  VLCB::CircularBuffer<int> buffer(4);
  int entry = 1;
  buffer.put(entry);
  entry = 2;
  buffer.put(entry);
  entry = 3;
  buffer.put(entry);
  entry = 4;
  buffer.put(entry);
  entry = 5;
  buffer.put(entry);

  assertEquals(true, buffer.available());
  assertEquals(4, buffer.getHighWaterMark());
  assertEquals(1, buffer.getOverflows());
  assertEquals(0, buffer.getNumberOfGets());
  assertEquals(5, buffer.getNumberOfPuts());
  
  // First entry has been overwritten.
  assertEquals(2, buffer.pop());
}

void testHeadWrapAround()
{
  test();
  
  VLCB::CircularBuffer<int> buffer(4);
  int entry = 1;
  buffer.put(entry);
  entry = 2;
  buffer.put(entry);

  assertEquals(1, buffer.pop());
  assertEquals(2, buffer.pop());

  entry = 3;
  buffer.put(entry);
  entry = 4;
  buffer.put(entry);
  entry = 5;
  buffer.put(entry);

  assertEquals(3, buffer.pop());
  assertEquals(4, buffer.pop());
  assertEquals(5, buffer.pop());

  assertEquals(false, buffer.available());
  assertEquals(3, buffer.getHighWaterMark());
  assertEquals(0, buffer.getOverflows());
  assertEquals(5, buffer.getNumberOfGets());
  assertEquals(5, buffer.getNumberOfPuts());
}

}

void testCircularBuffer()
{
  testEmpty();
  testHasOne();
  testFull();
  testOverflow();
  testHeadWrapAround();
}
