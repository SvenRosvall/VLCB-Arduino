//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

/*
  3rd party libraries needed for compilation: (not for binary-only distributions)

  Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
*/

//
// This is a modified version of VLCB_4in4out_slot
// It uses the ModifiedGridConnect protocol over serial instead of a CAN interface
// this allows a single module to connect via a USB cable to a management tool like FCU
// without a CAN network
//
///////////////////////////////////////////////////////////////////////////////////
// Pin Use map UNO:
// Digital pin 2
// Digital pin 3 (PWM)    Module LED 1
// Digital pin 4          VLCB Green LED
// Digital pin 5 (PWM)    Module LED 2
// Digital pin 6 (PWM)    Module LED 3
// Digital pin 7          VLCB Yellow LED
// Digital pin 8          VLCB Switch
// Digital pin 9 (PWM)    Module LED 4
// Digital pin 10
// Digital pin 11
// Digital pin 12
// Digital pin 13

// Digital / Analog pin 0     Module Switch 1
// Digital / Analog pin 1     Module Switch 2
// Digital / Analog pin 2     Module Switch 3
// Digital / Analog pin 3     Module Switch 4
// Digital / Analog pin 4     Not Used
// Digital / Analog pin 5     Not Used
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This module uses slots, i.e. event indices, for determining actions.
// The following slots are used:
// 1-4: When switch N is pressed/released send event in slot N.
// 10-49: When receiving an event that is stored in one of the slots N*10 ... N*10+9, trigger action for LED N.
//        There are 10 slots per LED so that there can be many events that activate the same LED.

// Node variables:
//  NV1-4 - Behaviour of switch 1-4: 0) None 1) On/Off 2) On only 3) Off only 4) Toggle

// There is only one event variable. It is used for consumed events to control the action
// for the LED: 0) No change 1) Normal 2) Slow blink 3) Fast blink

#define DEBUG 1  // set to 0 for no serial debug

#if DEBUG
#define DEBUG_PRINT(S) Serial << S << endl
#else
#define DEBUG_PRINT(S)
#endif

// 3rd party libraries
#include <Streaming.h>

// VLCB library header files
#include <VLCB.h>
#include <SerialGC.h>               // replaces CAN controller

// forward function declarations
void eventhandler(byte, const VLCB::VlcbMessage *);
void printConfig();
void processSwitches();

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'b';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 83;          // One higher than CAN4IN4OUT

// module name, must be at most 7 characters
char mname[] = "4I4O_S";

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// Module objects
const byte LED[] = {3, 5, 6, 9};     // LED pin connections through typ. 1K8 resistor
const byte SWITCH[] = {A0, A1, A2, A3}; // Module Switch takes input to 0V.

const byte NUM_LEDS = sizeof(LED) / sizeof(LED[0]);
const byte NUM_SWITCHES = sizeof(SWITCH) / sizeof(SWITCH[0]);


// instantiate module objects
VLCB::LED moduleLED[NUM_LEDS];     //  LED as output
VLCB::Switch moduleSwitch[NUM_SWITCHES];  //  switch as input
bool state[NUM_SWITCHES];

VLCB::SerialGC serialGC;                  // CAN transport object using serial

// Service objects
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&serialGC);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventSlotTeachingService etService;
VLCB::EventProducerService epService;

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  VLCB::checkStartupAction(LED_GRN, LED_YLW, SWITCH0);

  VLCB::setServices({
    &mnService, &ledUserInterface, &canService, &nvService,
    &ecService, &epService, &etService, &coeService});

  // set config layout parameters
  VLCB::setNumNodeVariables(NUM_SWITCHES);
  VLCB::setMaxEvents(64);
  VLCB::setNumEventVariables(1);

  // set module parameters
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BETA);
  VLCB::setModuleId(MANUFACTURER, MODULE_ID);

  // set module name
  VLCB::setName(mname);

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);
  // register the VLCB request event handler to receive event status requests.
  epService.setRequestEventHandler(eventhandler);

  // initialise and load configuration
  VLCB::begin();

  Serial << F("> mode = (") << _HEX(VLCB::getCurrentMode()) << ") " << VLCB::Configuration::modeString(VLCB::getCurrentMode());
  Serial << F(", CANID = ") << VLCB::getCANID();
  Serial << F(", NN = ") << VLCB::getNodeNum() << endl;

  // show code version and copyright notice
  printConfig();
}

void setupModule()
{
  // configure the module switches, active low
  for (byte i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].setPin(SWITCH[i], INPUT_PULLUP);
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
  VLCB::process();

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
    moduleSwitch[i].run();
    if (moduleSwitch[i].stateChanged())
    {
      byte nv = i + 1;
      byte nvval = VLCB::readNV(nv);
      byte swNum = i + 1;
      DEBUG_PRINT(F("sk> Button ") << swNum << F(" state change detected. NV Value = ") << nvval);

      if (nvval == 0)
      {
        DEBUG_PRINT(F("sk> No action for button."));
        continue;
      }

      byte eventSlot = swNum;
      if (!VLCB::doesEventExistAtIndex(eventSlot))
      {
        DEBUG_PRINT(F("sk> No event in this slot. Create a default event."));
        VLCB::createEventAtIndex(eventSlot, VLCB::getNodeNum(), swNum);
      }

      bool reportState = false;
      bool doReport = false;
      switch (nvval)
      {
        case 1:
          // ON and OFF
          reportState = state[i] = (moduleSwitch[i].isPressed());
          doReport = true;
          DEBUG_PRINT(F("sk> Button calling case 1: ") << swNum << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
          break;

        case 2:
          // Only ON
          if (moduleSwitch[i].isPressed()) 
          {
            reportState = state[i] = true;
            doReport = true;
            DEBUG_PRINT(F("sk> Button calling case 2: ") << swNum << F(" pressed, send state: ") << state[i]);
          }
          break;

        case 3:
          // Only OFF
          if (moduleSwitch[i].isPressed())
          {
            reportState = state[i] = false;
            doReport = true;
            DEBUG_PRINT(F("sk> Button calling case 3: ") << swNum << F(" pressed, send state: ") << state[i]);
          }
          break;

        case 4:
          // Toggle button
          if (moduleSwitch[i].isPressed())
          {
            reportState = state[i] = !state[i];
            doReport = true;
            DEBUG_PRINT(F("sk> Button calling case 4: ") << swNum << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
          }
          break;

        default:
          DEBUG_PRINT(F("sk> Button ") << swNum << F(" do nothing."));
          continue;
      }

      if (doReport)
      {
        DEBUG_PRINT(F("sk> Sending event at index=") << eventSlot);
        epService.sendEventAtIndex(reportState, eventSlot);
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

  byte ledNum = index / 10;
  if (ledNum == 0 || ledNum > NUM_LEDS)
  {
    DEBUG_PRINT(F("sk> event handler: Index ") << index << F(" is not for an LED slot."));
    return;
  }

  byte evval = VLCB::getEventEVval(index, 1);
  DEBUG_PRINT(F("sk> event handler: index = ") << index << F(" for LED = ") << ledNum); 
  DEBUG_PRINT(F("sk> opcode = 0x") << _HEX(msg->data[0]) << F(" = ") << opc);

  switch (opc)
  {
    case OPC_ACON:
    case OPC_ASON:
      DEBUG_PRINT(F("sk> case is opCode ON"));
      DEBUG_PRINT(F("sk> EV1 Value = ") << evval);

      switch (evval)
      {
        case 1:
          moduleLED[ledNum - 1].on();
          break;

        case 2:
          moduleLED[ledNum - 1].blink(500);
          break;

        case 3:
          moduleLED[ledNum - 1].blink(250);
          break;

        default:
          break;
      }
      break;

    case OPC_ACOF:
    case OPC_ASOF:
      DEBUG_PRINT(F("sk> case is opCode OFF"));
      if (evval > 0)
      {
        moduleLED[ledNum - 1].off();
      }
      break;
      
    // case OPC_AREQ:
    // case OPC_ASRQ:
    //   byte evval = VLCB::getEventEVval(index, 1);
    //   DEBUG_PRINT(F("> Handling request op =  ") << _HEX(opc) << F(", request input = ") << evval << F(", state = ") << state[evval]);
    //   epService.sendEventResponse(state[evval], index);
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
  Serial << F("> © Sven Rosvall (MERG 3777) 2025") << endl;
}
