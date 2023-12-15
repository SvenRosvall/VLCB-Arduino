//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
*/

// 3rd party libraries
#include <Streaming.h>

// VLCB library header files
#include <Controller.h>                   // Controller class
#include <CAN2515.h>               // CAN controller
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
#include "CombinedUserInterface.h"

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MODULE_ID = 99;          // VLCB module type

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// Controller objects
VLCB::Configuration modconfig;               // configuration object
VLCB::CAN2515 can2515;                  // CAN transport object
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::SerialUserInterface serialUserInterface(&modconfig, &can2515);
VLCB::CombinedUserInterface combinedUserInterface(&ledUserInterface, &serialUserInterface);
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;
VLCB::Controller controller(&combinedUserInterface, &modconfig, 
                            { &mnService, &canService, &nvService, &ecService, &epService, &etService, &coeService }); // Controller object

// module objects
VLCB::Switch moduleSwitch(5);            // an example switch as input
VLCB::LED moduleLED(6);                  // an example LED as output

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { '1', 'I', 'N', '1', 'O', 'U', 'T' };

// forward function declarations
byte checkInputProduced();
void eventhandler(byte, const VLCB::VlcbMessage *);
void processSerialInput();
void printConfig();
void processModuleSwitchChange();

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NVS_START = 10;
  modconfig.EE_NUM_NVS = 10;
  modconfig.EE_EVENTS_START = 20;
  modconfig.EE_MAX_EVENTS = 32;
  modconfig.EE_PRODUCED_EVENTS = 1;
  modconfig.EE_NUM_EVS = 1;
 

  // initialise and load configuration
  controller.begin();

  const char * modeString;
  switch (modconfig.currentMode)
  {
    case MODE_NORMAL: modeString = "Normal"; break;
    case MODE_SETUP: modeString = "Setup"; break;
    case MODE_UNINITIALISED: modeString = "Uninitialised"; break;
    default: modeString = "Unknown"; break;
  }
  Serial << F("> mode = (") << _HEX(modconfig.currentMode) << ") " << modeString;
  Serial << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  VLCB::Parameters params(modconfig);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setModuleId(MODULE_ID);
 
  // assign to Controller
  controller.setParams(params.getParams());
  controller.setName(mname);

  // module reset - if switch is depressed at startup and module is in Uninitialised mode
//  if (userInterface.isButtonPressed() && modconfig.currentMode == MODE_UNINITIALISED)
//  {
//    Serial << F("> switch was pressed at startup in Uninitialised mode") << endl;
//    modconfig.resetModule(&userInterface);
//  }

  // opportunity to set default NVs after module reset
  if (modconfig.isResetFlagSet())
  {
    Serial << F("> module has been reset") << endl;
    modconfig.clearResetFlag();
  }

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);

  // register check produced handler for assigning short and spoof codes
  etService.setcheckInputProduced(checkInputProduced);

  // set Controller LEDs to indicate mode
  controller.indicateMode(modconfig.currentMode);

  // configure and start CAN bus and VLCB message processing
  can2515.setNumBuffers(2, 1);      // more buffers = more memory used, fewer = less
  can2515.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module
  can2515.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections
  if (!can2515.begin())
  {
    Serial << F("> error starting VLCB") << endl;
  }
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

  //
  /// check CAN message buffers
  //
  if (can2515.canp->receiveBufferPeakCount() > can2515.canp->receiveBufferSize())
  {
    Serial << F("> receive buffer overflow") << endl;
  }

  if (can2515.canp->transmitBufferPeakCount(0) > can2515.canp->transmitBufferSize(0))
  {
    Serial << F("> transmit buffer overflow") << endl;
  }

  //
  /// check CAN bus state
  //
  byte s = can2515.canp->errorFlagRegister();
  if (s != 0)
  {
    Serial << F("> error flag register is non-zero") << endl;
  }

  // bottom of loop()
}

//
// Callback used when teaching produced events. 
// Returns the index of the switch that was pressed which is the taught event shall relate to.
//
byte checkInputProduced()
{
  moduleSwitch.run();
  if (moduleSwitch.stateChanged())
  {
    // Button was pressed so event is for it. Index 0.
    return 0;
  }
  else
  {
    // No button pressed. Event is for consumed event.
    return 0xFF;
  }
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
    byte eventIndex = 0;  
    epService.sendEvent(state, eventIndex);
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

  byte evval = modconfig.getEventEVval(index, 1);
  // Event Off op-codes have odd numbers.
  bool ison = (msg->data[0] & 0x01) == 0;

  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;

  // read the value of the first event variable (EV) associated with this learned event
  Serial << F("> EV1 = ") << evval << endl;

  // set the LED according to the opcode of the received event, if the first EV equals 0
  // we turn on the LED and if the first EV equals 1 we use the blink() method of the LED object as an example
  if (ison)
  {
    if (evval == 0)
    {
      Serial << F("> switching the LED on") << endl;
      moduleLED.on();
    }
    else if (evval == 1)
    {
      Serial << F("> switching the LED to blink") << endl;
      moduleLED.blink();
    }
  }
  else
  {
    Serial << F("> switching the LED off") << endl;
    moduleLED.off();
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
