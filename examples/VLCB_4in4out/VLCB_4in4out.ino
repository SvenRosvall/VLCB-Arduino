// VLCB_4in4out

/*
  Copyright (C) 2023 Martin Da Costa
  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

  3rd party libraries needed for compilation: (not for binary-only distributions)

  Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
  ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
*/

///////////////////////////////////////////////////////////////////////////////////
// Pin Use map UNO:
// Digital pin 2          Interupt CAN
// Digital pin 3 (PWM)    Module LED 1
// Digital pin 4          VLCB Green LED
// Digital pin 5 (PWM)    Module LED 2
// Digital pin 6 (PWM)    Module LED 3
// Digital pin 7          VLCB Yellow LED
// Digital pin 8          VLCB Switch
// Digital pin 9 (PWM)    Module LED 4
// Digital pin 10 (SS)    CS    CAN
// Digital pin 11 (MOSI)  SI    CAN
// Digital pin 12 (MISO)  SO    CAN
// Digital pin 13 (SCK)   Sck   CAN

// Digital / Analog pin 0     Module Switch 1
// Digital / Analog pin 1     Module Switch 2
// Digital / Analog pin 2     Module Switch 3
// Digital / Analog pin 3     Module Switch 4
// Digital / Analog pin 4     Not Used
// Digital / Analog pin 5     Not Used
//////////////////////////////////////////////////////////////////////////

#define DEBUG 1  // set to 0 for no serial debug

#if DEBUG
#define DEBUG_PRINT(S) Serial << S << endl
#else
#define DEBUG_PRINT(S)
#endif

// 3rd party libraries
#include <Streaming.h>
#include <Bounce2.h>

// VLCB library header files
#include <Controller.h>                   // Controller class
#include <CAN2515.h>               // CAN controller
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

#include "LEDControl.h"

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 82;          // VLCB module type

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// Controller objects
VLCB::Configuration modconfig;               // configuration object
VLCB::CAN2515 can2515;                  // CAN transport object
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::SerialUserInterface serialUserInterface(&can2515);
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;
VLCB::Controller controller(&modconfig,
                            {&mnService, &ledUserInterface, &serialUserInterface, &canService, &nvService, &ecService, &epService, &etService, &coeService}); // Controller object

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { '4', 'I', 'N', '4', 'O', 'U', 'T' };

// Module objects
const byte LED[] = {3, 5, 6, 9};     // LED pin connections through typ. 1K8 resistor
const byte SWITCH[] = {A0, A1, A2, A3}; // Module Switch takes input to 0V.

const byte NUM_LEDS = sizeof(LED) / sizeof(LED[0]);
const byte NUM_SWITCHES = sizeof(SWITCH) / sizeof(SWITCH[0]);

// module objects
Bounce moduleSwitch[NUM_SWITCHES];  //  switch as input
LEDControl moduleLED[NUM_LEDS];     //  LED as output
bool state[NUM_SWITCHES];

// forward function declarations
void eventhandler(byte, const VLCB::VlcbMessage *);
void printConfig();
void processSwitches();

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NUM_NVS = NUM_SWITCHES;
  modconfig.EE_EVENTS_START = 50;
  modconfig.EE_MAX_EVENTS = 64;
  modconfig.EE_PRODUCED_EVENTS = NUM_SWITCHES;
  modconfig.EE_NUM_EVS = 1 + NUM_LEDS;

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
  // register the VLCB request event handler to receive event status requests.
  epService.setRequestEventHandler(eventhandler);

  // set Controller LEDs to indicate the current mode
  controller.indicateMode(modconfig.currentMode);

  // configure and start CAN bus and VLCB message processing
  can2515.setNumBuffers(6, 1);      // more buffers = more memory used, fewer = less
  can2515.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module
  can2515.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections
  if (!can2515.begin())
  {
    Serial << F("> error starting VLCB") << endl;
  }

  // initialise and load configuration
  controller.begin();

  Serial << F("> mode = ") << VLCB::Configuration::modeString(modconfig.currentMode) << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;

  // show code version and copyright notice
  printConfig();
}

void setupModule()
{
  // configure the module switches, active low
  for (byte i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].attach(SWITCH[i], INPUT_PULLUP);
    moduleSwitch[i].interval(5);
    state[i] = false;
  }

  // configure the module LEDs
  for (byte i = 0; i < NUM_LEDS; i++)
  {
    moduleLED[i].setPin(LED[i]);
  }

  Serial << "> Module has " << NUM_LEDS << " LEDs and " << NUM_SWITCHES << " switches." << endl;
}

//
/// setup - runs once at power on
//
void setup()
{
  Serial.begin (115200);
  Serial << endl << endl << F("> ** VLCB Arduino basic example module ** ") << __FILE__ << endl;

  setupVLCB();
  setupModule();

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

  // Run the LED code
  for (int i = 0; i < NUM_LEDS; i++)
  {
    moduleLED[i].run();
  }

  // test for switch input
  processSwitches();

  // bottom of loop()
}

void processSwitches(void) 
{
  for (byte i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].update();
    if (moduleSwitch[i].changed())
    {
      byte nv = i + 1;
      byte nvval = modconfig.readNV(nv);
      byte swNum = i + 1;

      // DEBUG_PRINT(F("sk> Button ") << i << F(" state change detected. NV Value = ") << nvval);

      switch (nvval)
      {
        case 1:
          // ON and OFF
          state[i] = (moduleSwitch[i].fell());
          //DEBUG_PRINT(F("sk> Button ") << i << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
          epService.sendEvent(state[i], swNum);
          break;

        case 2:
          // Only ON
          if (moduleSwitch[i].fell()) 
          {
            state[i] = true;
            //DEBUG_PRINT(F("sk> Button ") << i << F(" pressed, send state: ") << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        case 3:
          // Only OFF
          if (moduleSwitch[i].fell())
          {
            state[i] = false;
            //DEBUG_PRINT(F("sk> Button ") << i << F(" pressed, send state: ") << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        case 4:
          // Toggle button
          if (moduleSwitch[i].fell())
          {
            state[i] = !state[i];
            //DEBUG_PRINT(F("sk> Button ") << i << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        default:
          //DEBUG_PRINT(F("sk> Button ") << i << F(" do nothing."));
          break;
      }
    }
  }
}

//
/// called from the VLCB library when a learned event is received
//
void eventhandler(byte index, const VLCB::VlcbMessage *msg)
{
  byte opc = msg->data[0];

  //DEBUG_PRINT(F("sk> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]));

  unsigned int node_number = (msg->data[1] << 8) + msg->data[2];
  unsigned int event_number = (msg->data[3] << 8) + msg->data[4];
  //DEBUG_PRINT(F("sk> NN = ") << node_number << F(", EN = ") << event_number);
  //DEBUG_PRINT(F("sk> op_code = ") << opc);

  switch (opc) 
  {
    case OPC_ACON:
    case OPC_ASON:
      //DEBUG_PRINT(F("sk> case is opCode ON"));
      for (byte i = 0; i < NUM_LEDS; i++)
      {
        byte ev = i + 2;
        byte evval = modconfig.getEventEVval(index, ev);
        //DEBUG_PRINT(F("sk> EV = ") << ev << (" Value = ") << evval);

        switch (evval) 
        {
          case 1:
            moduleLED[i].on();
            break;

          case 2:
            moduleLED[i].flash(500);
            break;

          case 3:
            moduleLED[i].flash(250);
            break;

          default:
            break;
        }
      }
      break;

    case OPC_ACOF:
    case OPC_ASOF:
    //DEBUG_PRINT(F("sk> case is opCode OFF"));
      for (byte i = 0; i < NUM_LEDS; i++)
      {
        byte ev = i + 2;
        byte evval = modconfig.getEventEVval(index, ev);

        if (evval > 0)
        {
          moduleLED[i].off();
        }
      }
      break;
      
    case OPC_AREQ:
    case OPC_ASRQ:
      byte evval = modconfig.getEventEVval(index, 1) - 1;
      DEBUG_PRINT(F("> Handling request op =  ") << _HEX(opc) << F(", request input = ") << evval << F(", state = ") << state[evval]);
      epService.sendEventResponse(state[evval], index);
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
  Serial << F("> © Martin Da Costa (MERG M6237) 2023") << endl;
}
