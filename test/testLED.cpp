//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "TestTools.hpp"
#include "ArduinoMock.hpp"

#include "LED.h"

namespace
{

void testCreate()
{
  test();
  VLCB::LED led(7);

  assertEquals(false, led.getState());
}

void testOn()
{
  test();
  VLCB::LED led(7);
  
  led.on();
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testOff()
{
  test();
  VLCB::LED led(7);
  
  led.off();
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testPulse()
{
  test();
  VLCB::LED led(7);
  
  led.pulse();
  led.run();
  
// State is not maintained this way for pulsing.
//  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  addMillis(11);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));

  // Ensure it stays low
  addMillis(11);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testOnThenPulse()
{
  test();
  VLCB::LED led(7);

  led.on();
  led.run();

  addMillis(100);
  led.pulse();
  led.run();

  assertEquals(HIGH, getDigitalWrite(7));
  
  // Ensure LED is off after pulsing despite the previous on() call.
  addMillis(20);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testPulseThenOff()
{
  test();
  VLCB::LED led(7);

  led.pulse();
  led.run();

  addMillis(2);
  led.off();
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
  
  // Ensure the LED stays off
  addMillis(10);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testPulseThenOn()
{
  test();
  VLCB::LED led(7);

  led.pulse();
  led.run();

  addMillis(2);
  led.on();
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
  
  // Ensure the LED stays on
  addMillis(10);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testBlink()
{
  test();
  VLCB::LED led(7);
  
  led.blink();
  led.run();
  
  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  addMillis(501);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));

  // And goes back on again
  addMillis(500);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testOnThenBlink()
{
  test();
  VLCB::LED led(7);

  led.on();
  led.run();

  led.blink();
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
  
  // Ensure LED is blinking.
  addMillis(501);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testBlinkThenOff()
{
  test();
  VLCB::LED led(7);

  led.blink();
  led.run();

  addMillis(2);
  led.off();
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
  
  // Ensure the LED stays off
  addMillis(10);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testBlinkThenOn()
{
  test();
  VLCB::LED led(7);

  led.blink();
  led.run();

  addMillis(2);
  led.on();
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
  
  // Ensure the LED stays on
  addMillis(10);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testBlinkThenPulse()
{
  test();
  VLCB::LED led(7);

  led.blink();
  led.run();

  addMillis(2);
  led.pulse();
  led.run();

//  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  addMillis(10);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
  
  // Ensure the LED stays off.
  addMillis(500);
  led.run();

  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

void testPulseThenBlink()
{
  test();
  VLCB::LED led(7);

  led.pulse();
  led.run();

  addMillis(2);
  led.blink();
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  // Ensure the pulse code is not in effect
  addMillis(10);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testLongPulseThenShortPulse()
{
  test();
  VLCB::LED led(7);

  led.pulse(500);
  led.run();

  addMillis(2);
  led.pulse(10);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  // Ensure the long pulse is still in effect
  addMillis(12);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));
}

void testShortPulseThenLongPulse()
{
  test();
  VLCB::LED led(7);

  led.pulse(10);
  led.run();

  addMillis(2);
  led.pulse(500);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  // Ensure the long pulse is still in effect
  addMillis(12);
  led.run();

  assertEquals(true, led.getState());
  assertEquals(HIGH, getDigitalWrite(7));

  addMillis(490);
  led.run();

  // Long pulse has ended.
  assertEquals(false, led.getState());
  assertEquals(LOW, getDigitalWrite(7));
}

}

void testLED()
{
  testCreate();
  testOn();
  testOff();
  testPulse();
  testOnThenPulse();
  testPulseThenOff();
  testPulseThenOn();
  testBlink();
  testOnThenBlink();
  testBlinkThenOff();
  testBlinkThenOn();
  testBlinkThenPulse();
  testPulseThenBlink();
  testLongPulseThenShortPulse();
  testShortPulseThenLongPulse();
}
