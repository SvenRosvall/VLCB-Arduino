//  Copyright (C) Bruno Rocci (bruno_rocci@hotmail.com)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming      -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      SoftwareSerial -- Arduino software serial library (not needed when USE_MEGA_SERIAL1 is set)
*/

// This example demonstrates split serial usage:
//
// - SerialGC uses the default hardware Serial port for GridConnect transport.
// - SerialUserInterface uses SoftwareSerial for user commands and status output.
// - On Mega2560, hardware Serial1 can optionally be used for SerialUserInterface.
//   This is controlled by the USE_MEGA_SERIAL1 macro, which can be defined
//   either in the sketch (Arduino IDE) or as a build flag (e.g. in platformio.ini).
//
// To allow concurrent use of SerialGC and SerialUserInterface, the library lets
// users assign each class to different serial interfaces in their declarations.
//
// This is useful on boards where the USB serial port is already dedicated to
// VLCB/GridConnect transport, but a separate command console is still required.
//
// Any Stream-compatible serial port can be used for the SerialUserInterface, but
// SoftwareSerial is used in this example for compatibility with a wide range of
// boards. Note that SoftwareSerial has some limitations, such as not being able
// to receive data while transmitting, and may not be suitable for high-speed
// communication. It is not recommended to use SoftwareSerial for the SerialGC
// transport, as it may not be able to keep up with the data rate and could lead
// to data loss. For better performance, consider using a board with multiple
// hardware serial ports and connecting the SerialUserInterface to one of those.
//
// Based on the VLCB_SerialGC_empty example from Sven Rosvall (MERG 3777).

// 3rd party libraries
#include <Streaming.h>
#ifndef USE_MEGA_SERIAL1
#include <SoftwareSerial.h>
#endif

// VLCB library header files
#include <VLCB.h>
#include <SerialGC.h>
#include <SerialUserInterface.h>

// forward function declarations
void printConfig();

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BUILD = 0;           // code build number
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 98;          // VLCB module type

// module name, must be at most 7 characters
char mname[] = "SPLIT";

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

#ifdef USE_MEGA_SERIAL1
// Use hardware Serial1 on pins 18 (TX1) and 19 (RX1)
#define serialUserPort Serial1
#else
// Use SoftwareSerial on pins 2 (RX) and 3 (TX)
const byte UI_RX_PIN = 2;
const byte UI_TX_PIN = 3;
SoftwareSerial serialUserPort(UI_RX_PIN, UI_TX_PIN);
#endif

// module objects
VLCB::SerialGC serialGC(Serial);                                // CAN transport object using hardware serial
VLCB::SerialUserInterface serialUserInterface(serialUserPort);  // User interface object using either SoftwareSerial or Serial1, depending on USE_MEGA_SERIAL1

// Service objects
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::MinimumNodeServiceWithDiagnostics mnService;
VLCB::CanService serialCanService(&serialGC);

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  VLCB::checkStartupAction(LED_GRN, LED_YLW, SWITCH0);

  VLCB::setServices({
    &mnService, &ledUserInterface, &serialUserInterface, &serialCanService});

  // set module parameters
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BUILD);
  VLCB::setModuleId(MANUFACTURER, MODULE_ID);

  // set module name
  VLCB::setName(mname);

  // initialise and load configuration
  VLCB::begin();

  // show code version and copyright notice on the UI console
  serialUserPort << F("> mode = ") << VLCB::Configuration::modeString(VLCB::getCurrentMode());
  serialUserPort << F(", CANID = ") << VLCB::getCANID();
  serialUserPort << F(", NN = ") << VLCB::getNodeNum() << endl;

  printConfig();
}

//
/// setup - runs once at power on
//
void setup()
{
  Serial.begin(115200);
  serialUserPort.begin(9600);
  delay(2000);  // Give some time to PIO to open the serial monitor before printing anything

#ifdef USE_MEGA_SERIAL1
  serialUserPort << F("> ** VLCB split: SerialGC + Serial1 UI example ** ") << __FILE__ << endl;
#else
  serialUserPort << F("> ** VLCB split: SerialGC + SoftwareSerial UI example ** ") << __FILE__ << endl;
#endif

  setupVLCB();

  serialUserPort << F("> ready") << endl << endl;
}

//
/// loop - runs forever
//
void loop()
{
  //
  /// do VLCB message, switch and LED processing
  //
  VLCB::process();

  // bottom of loop()
}

//
/// print code version config details and copyright notice
//
void printConfig()
{
  // code version
  serialUserPort << F("> code version = ") << VER_MAJ << VER_MIN << F(" build ") << VER_BUILD << endl;
  serialUserPort << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  serialUserPort << F("> Copyright © 2026 Bruno Rocci (MERG 9690)") << endl;
}
