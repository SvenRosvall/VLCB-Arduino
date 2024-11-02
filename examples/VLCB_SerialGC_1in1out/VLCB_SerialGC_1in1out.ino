//  Copyright (C) David Ellis (david@ellis128.co.uk)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
*/


//
// This is a modified version of VLCB_1in1out
// It uses the ModifiedGridConnect protocol over serial instead of a CAN interface
// this allows a single module to connect via a USB cable to a management tool like FCU
// without a CAN network
//


// 3rd party libraries
#include <Streaming.h>

// VLCB library header files
#include <Controller.h>                   // Controller class
#include <SerialGC.h>               // replaces CAN controller
#include <Switch.h>             // pushbutton switch
#include <LED.h>                // VLCB LEDs
#include <Configuration.h>             // module configuration
#include <Parameters.h>             // VLCB parameters
#include <vlcbdefs.hpp>               // VLCB constants
#include <LEDUserInterface.h>
#include "MinimumNodeService.h"
#include "CanService.h"
#include "NodeVariableService.h"
#include "EventConsumerService.h"
#include "EventProducerService.h"
#include "ConsumeOwnEventsService.h"
#include "EventTeachingService.h"
#include "SerialUserInterface.h"

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 99;          // VLCB module type

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// Controller objects
VLCB::Configuration modconfig;               // configuration object
VLCB::SerialGC serialGC;                  // CAN transport object using serial
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&serialGC);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;
VLCB::Controller controller(&modconfig, 
                            {&mnService, &ledUserInterface, &canService, &nvService, &ecService, &epService, &etService, &coeService}); // Controller object

// module objects
VLCB::Switch moduleSwitch(5);            // an example switch as input
VLCB::LED moduleLED(6);                  // an example LED as output

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { '1', 'I', 'N', '1', 'O', 'U', 'T' };

// forward function declarations
void eventhandler(byte, const VLCB::VlcbMessage *);
void printConfig();
void processModuleSwitchChange();

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NUM_NVS = 10;
  modconfig.EE_MAX_EVENTS = 32;
  modconfig.EE_PRODUCED_EVENTS = 1;
  modconfig.EE_NUM_EVS = 2; // EV1: Produced event ; EV2: LED1
 
  // set module parameters
  VLCB::Parameters params(modconfig);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setManufacturer(MANUFACTURER);
  params.setModuleId(MODULE_ID);  
 
  // assign to Controller
  controller.setParams(params.getParams());
  controller.setName(mname);

  // module reset - if switch is depressed at startup
  if (ledUserInterface.isButtonPressed())
  {
    Serial << F("> switch was pressed at startup") << endl;
    modconfig.resetModule();
  }

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);

  // initialise and load configuration
  controller.begin();

  Serial << F("> mode = (") << _HEX(modconfig.currentMode) << ") " << VLCB::Configuration::modeString(modconfig.currentMode);
  Serial << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;

  // show code version and copyright notice
  printConfig();
}

//
/// setup - runs once at power on
//
void setup()
{
  Serial.begin (115200);
  Serial << endl << endl << F("> ** VLCB 1 in 1 out v1 ** ") << __FILE__ << endl;

  setupVLCB();

  // end of setup
  Serial << F("> ready") << endl << endl;
}

//
/// loop - runs forever
//
void loop()
{
  //
  /// do VLCB message, switch and LED processing
  //
  controller.process();

  //
  /// give the switch and LED code some time to run
  //
  moduleSwitch.run();
  moduleLED.run();

  //
  /// Check if smich changed and do any processing for this change.
  //
  processModuleSwitchChange();

  // bottom of loop()
}

//
/// test for switch input
/// as an example, it must be have been pressed or released for at least half a second
/// then send a long VLCB event with opcode ACON for on and ACOF for off

/// you can just watch for this event in FCU or JMRI, or teach it to another VLCB consumer module
//
void processModuleSwitchChange()
{
  if (moduleSwitch.stateChanged())
  {
    bool state = moduleSwitch.isPressed();
    byte inputChannel = 1;  
    epService.sendEvent(state, inputChannel);
  }
}

//
/// user-defined event processing function
/// called from the VLCB library when a learned event is received
/// it receives the event table index and the CAN frame
//
void eventhandler(byte index, const VLCB::VlcbMessage *msg)
{
  // as an example, control an LED

  byte evval = modconfig.getEventEVval(index, 2);  //read ev2 because ev1 defines producer.
  // Event Off op-codes have odd numbers.
  bool ison = (msg->data[0] & 0x01) == 0;

  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;
  Serial << F("> EV2 = ") << evval << endl;

  // set the LED according to the opcode of the received event, if the second EV equals 1
  // we turn on the LED and if the first EV equals 2 we use the blink() method of the LED object as an example.
  if (ison)
  {
    switch (evval)
    {
      case 1:
        Serial << F("> switching the LED on") << endl;
        moduleLED.on();
        break;

      case 2:
        Serial << F("> switching the LED to blink") << endl;
        moduleLED.blink();
        break;
    }
  }
  else
  {
    if (evval > 0)
    {
      Serial << F("> switching the LED off") << endl;
      moduleLED.off();
    }
  }
}

//
/// print code version config details and copyright notice
//
void printConfig()
{
  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Duncan Greenwood (MERG M5767) 2019") << endl;
}
