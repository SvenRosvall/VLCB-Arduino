// VLCB_4in4out

/*
  Copyright (C) 2023 Martin Da Costa
  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

  3rd party libraries needed for compilation: (not for binary-only distributions)

  Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
  ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
  Bounce2  -- library for switch debounce.
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

// VLCB library header files
#include <VLCB.h>
#include <CAN2515.h>               // Chosen CAN controller

// forward function declarations
void eventhandler(byte, const VLCB::VlcbMessage *);
void printConfig();
void processSwitches();

// constants
const byte VER_MAJ = 2;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 82;          // VLCB module type

const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// module name, must be 7 characters, space padded.
char mname[] = "4IN4OUT";

// Module objects
const byte LED[] = {3, 5, 6, 9};     // LED pin connections through typ. 1K8 resistor
const byte SWITCH[] = {A0, A1, A2, A3}; // Module Switch takes input to 0V.

const byte NUM_LEDS = sizeof(LED) / sizeof(LED[0]);
const byte NUM_SWITCHES = sizeof(SWITCH) / sizeof(SWITCH[0]);


// instantiate module objects
VLCB::Switch moduleSwitch[NUM_SWITCHES];  //  switch as input
VLCB::LED moduleLED[NUM_LEDS];     //  LED as output
bool state[NUM_SWITCHES];

VLCB::CAN2515 can2515;                  // CAN transport object

// Service objects
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::SerialUserInterface serialUserInterface;
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  VLCB::checkStartupAction(LED_GRN, LED_YLW, SWITCH0);

  VLCB::setServices({
    &mnService, &ledUserInterface, &serialUserInterface, &canService, &nvService,
    &ecService, &epService, &etService, &coeService});
  // set config layout parameters
  VLCB::setNumNodeVariables(NUM_SWITCHES);
  VLCB::setEventsStart(50);
  VLCB::setMaxEvents(64);
  VLCB::setNumProducedEvents(NUM_SWITCHES);
  VLCB::setNumEventVariables(1 + NUM_LEDS);

  // set module parameters
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BETA);
  VLCB::setModuleId(MANUFACTURER, MODULE_ID);

  // set module name
  VLCB::setName(mname);

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);
  // register the VLCB request event handler to receive event status requests.
  epService.setRequestEventHandler(eventhandler);

  // configure and start CAN bus and VLCB message processing
  can2515.setNumBuffers(6, 1);      // more buffers = more memory used, fewer = less
  can2515.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module
  can2515.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections
  if (!can2515.begin())
  {
    Serial << F("> error starting VLCB") << endl;
  }

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
    moduleLED[i].setPin(LED[i], LOW);  //Second arguement active low or active high. Default if no second arguement is active low.
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

      DEBUG_PRINT(F("sk> Button ") << i << F(" state change detected. NV Value = ") << nvval);

      switch (nvval)
      {
        case 1:
          // ON and OFF
          state[i] = (moduleSwitch[i].getState());
          DEBUG_PRINT(F("sk> Button ") << i << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
          epService.sendEvent(state[i], swNum);
          break;

        case 2:
          // Only ON
          if (moduleSwitch[i].isPressed()) 
          {
            state[i] = true;
            DEBUG_PRINT(F("sk> Button ") << i << F(" pressed, send state: ") << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        case 3:
          // Only OFF
          if (moduleSwitch[i].isPressed())
          {
            state[i] = false;
            DEBUG_PRINT(F("sk> Button ") << i << F(" pressed, send state: ") << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        case 4:
          // Toggle button
          if (moduleSwitch[i].isPressed())
          {
            state[i] = !state[i];
            DEBUG_PRINT(F("sk> Button ") << i << (state[i] ? F(" pressed, send state: ") : F(" released, send state: ")) << state[i]);
            epService.sendEvent(state[i], swNum);
          }
          break;

        default:
          DEBUG_PRINT(F("sk> Button ") << i << F(" do nothing."));
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

  DEBUG_PRINT(F("sk> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]));

  unsigned int node_number = (msg->data[1] << 8) + msg->data[2];
  unsigned int event_number = (msg->data[3] << 8) + msg->data[4];
  DEBUG_PRINT(F("sk> NN = ") << node_number << F(", EN = ") << event_number);
  DEBUG_PRINT(F("sk> op_code = ") << opc);

  switch (opc) 
  {
    case OPC_ACON:
    case OPC_ASON:
      DEBUG_PRINT(F("sk> case is opCode ON"));
      for (byte i = 0; i < NUM_LEDS; i++)
      {
        byte ev = i + 2;
        byte evval = VLCB::getEventEVval(index, ev);
        DEBUG_PRINT(F("sk> EV = ") << ev << (" Value = ") << evval);

        switch (evval) 
        {
          case 1:
            moduleLED[i].on();
            break;

          case 2:
            moduleLED[i].blink(500);
            break;

          case 3:
            moduleLED[i].blink(250);
            break;

          default:
            break;
        }
      }
      break;

    case OPC_ACOF:
    case OPC_ASOF:
    DEBUG_PRINT(F("sk> case is opCode OFF"));
      for (byte i = 0; i < NUM_LEDS; i++)
      {
        byte ev = i + 2;
        byte evval = VLCB::getEventEVval(index, ev);

        if (evval > 0)
        {
          moduleLED[i].off();
        }
      }
      break;
      
    case OPC_AREQ:
    case OPC_ASRQ:
      byte evval = VLCB::getEventEVval(index, 1) - 1;
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
