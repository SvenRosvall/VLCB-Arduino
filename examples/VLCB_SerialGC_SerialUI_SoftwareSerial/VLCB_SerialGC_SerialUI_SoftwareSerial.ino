//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming      -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      SoftwareSerial -- Arduino software serial library
*/

//
// This example demonstrates split serial usage:
// - SerialGC uses the hardware Serial port for GridConnect transport
// - SerialUserInterface uses SoftwareSerial for user commands and status output
//
// This is useful on boards where the USB serial port is already committed to
// the VLCB/GridConnect transport, but a separate command console is still wanted.
//


// 3rd party libraries
#include <Streaming.h>
#include <SoftwareSerial.h>

// VLCB library header files
#include <VLCB.h>
#include <SerialGC.h>
#include <SerialUserInterface.h>

// forward function declarations
void printConfig();

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 98;          // VLCB module type

// module name, must be at most 7 characters
char mname[] = "SPLIT";

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// SoftwareSerial pins: RX, TX
// Wire the external serial terminal or adapter to these pins.
const byte UI_RX_PIN = 10;
const byte UI_TX_PIN = 11;
SoftwareSerial serialUserPort(UI_RX_PIN, UI_TX_PIN);

// module objects
VLCB::SerialGC serialGC(Serial);                 // CAN transport object using hardware serial
VLCB::SerialUserInterface serialUserInterface(serialUserPort);

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
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BETA);
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

  serialUserPort << F("> ** VLCB split SerialGC + SoftwareSerial UI example ** ") << __FILE__ << endl;

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
  serialUserPort << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  serialUserPort << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  serialUserPort << F("> © Sven Rosvall (MERG 3777) 2026") << endl;
}
