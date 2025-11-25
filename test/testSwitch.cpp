//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "TestTools.hpp"
#include "ArduinoMock.hpp"

#include "Switch.h"

namespace
{

void testCreate()
{
  test();
  setDigitalRead(7, HIGH);
  
  VLCB::Switch aSwitch(7);

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
  
  aSwitch.run();

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

void testCreateHIGH()
{
  test();
  setDigitalRead(7, LOW);
  
  VLCB::Switch aSwitch(7, HIGH);

  assertEquals(LOW, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());

  aSwitch.run();

  assertEquals(LOW, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

void testSetPin()
{
  test();
  setDigitalRead(7, HIGH);
  
  VLCB::Switch aSwitch;
  aSwitch.setPin(7, INPUT_PULLUP);

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
  
  aSwitch.run();

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

void testSetPinHIGH()
{
  test();
  setDigitalRead(7, LOW);
  
  VLCB::Switch aSwitch;
  aSwitch.setPin(7, INPUT);

  assertEquals(LOW, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());

  aSwitch.run();

  assertEquals(LOW, aSwitch.getState());
  assertEquals(false, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

void testSwitchChanged()
{
  test();
  setDigitalRead(7, HIGH);
  VLCB::Switch aSwitch(7);
  aSwitch.run();

  setDigitalRead(7, LOW);
  aSwitch.run();

  assertEquals(LOW, aSwitch.getState());
  assertEquals(true, aSwitch.stateChanged());
  assertEquals(true, aSwitch.isPressed());
  
  aSwitch.run();
  assertEquals(false, aSwitch.stateChanged());

  setDigitalRead(7, HIGH);
  aSwitch.run();

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(true, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

void testSwitchChangedHIGH()
{
  test();
  setDigitalRead(7, LOW);
  VLCB::Switch aSwitch(7, HIGH);
  aSwitch.run();

  setDigitalRead(7, HIGH);
  aSwitch.run();

  assertEquals(HIGH, aSwitch.getState());
  assertEquals(true, aSwitch.stateChanged());
  assertEquals(true, aSwitch.isPressed());
  
  aSwitch.run();
  assertEquals(false, aSwitch.stateChanged());

  setDigitalRead(7, LOW);
  aSwitch.run();

  assertEquals(LOW, aSwitch.getState());
  assertEquals(true, aSwitch.stateChanged());
  assertEquals(false, aSwitch.isPressed());
}

}

void testSwitch()
{
  testCreate();
  testCreateHIGH();
  testSetPin();
  testSetPinHIGH();
  testSwitchChanged();
  testSwitchChangedHIGH();
}