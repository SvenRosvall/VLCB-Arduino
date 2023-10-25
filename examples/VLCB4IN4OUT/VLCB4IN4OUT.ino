
// VLCB4in4out


/*
  Copyright (C) 2023 Martin Da Costa
  //  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
*/

/*
      3rd party libraries needed for compilation:

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
#include <Switch.h>             // pushbutton switch
#include <LED.h>                // VLCB LEDs
#include <Configuration.h>             // module configuration
#include <Parameters.h>             // VLCB parameters
#include <vlcbdefs.hpp>               // VLCB constants
#include <LEDUserInterface.h>
#include "MinimumNodeService.h"
#include "NodeVariableService.h"
#include "EventConsumerService.h"
#include "EventProducerService.h"
#include "EventTeachingService.h"
#include "CanService.h"

#include "LEDControl.h"

////////////DEFINE MODULE/////////////////////////////////////////////////

// module name
unsigned char mname[7] = { '4', 'I', 'N', '4', 'O', 'U', 'T' };

// constants
const byte VER_MAJ = 1;     // code major version
const char VER_MIN = 'a';   // code minor version
const byte VER_BETA = 0;    // code beta sub-version
const byte MODULE_ID = 98;  // VLCB module type

const byte LED_GRN = 4;             // VLCB green Uninitialised LED pin
const byte LED_YLW = 7;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 8;             // VLCB push button switch pin

// Controller objects
VLCB::LEDUserInterface userInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::Configuration modconfig;               // configuration object
VLCB::CAN2515 can2515;                  // CAN transport object
VLCB::MinimumNodeService mnService;
VLCB::CanService canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;
VLCB::Controller controller(&userInterface, &modconfig, &can2515, 
                            { &mnService, &canService, &nvService, &etService, &epService, &ecService }); // Controller object

// Module objects
const byte LED[] = {3, 5, 6, 9};     // LED pin connections through typ. 1K8 resistor
const byte SWITCH[] = {A0, A1, A2 , A3}; // Module Switch takes input to 0V.

const byte NUM_LEDS = sizeof(LED) / sizeof(LED[0]);
const byte NUM_SWITCHES = sizeof(SWITCH) / sizeof(SWITCH[0]);

// module objects
Bounce moduleSwitch[NUM_SWITCHES];  //  switch as input
LEDControl moduleLED[NUM_LEDS];     //  LED as output
byte switchState[NUM_SWITCHES];

// forward function declarations
void eventhandler(byte, VLCB::CANFrame *);
void processSerialInput(void);
void printConfig(void);
void processModuleSwitchChange(void);

//
///  setup VLCB - runs once at power on called from setup()
//
void setupVLCB()
{
  // set config layout parameters
  modconfig.EE_NVS_START = 10;
  modconfig.EE_NUM_NVS = NUM_SWITCHES;
  modconfig.EE_EVENTS_START = 50;
  modconfig.EE_MAX_EVENTS = 64;
  modconfig.EE_PRODUCED_EVENTS = 4;
  modconfig.EE_NUM_EVS = NUM_LEDS;
  
  // initialise and load configuration
  controller.begin();

  Serial << F("> mode = ") << ((modconfig.currentMode) ? "Normal" : "Uninitialised") << F(", CANID = ") << modconfig.CANID;
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
  if (userInterface.isButtonPressed() && modconfig.currentMode == MODE_UNINITIALISED)
  {
    Serial << F("> switch was pressed at startup in Uninitialised mode") << endl;
    modconfig.resetModule(&userInterface);
  }

  // opportunity to set default NVs after module reset
  if (modconfig.isResetFlagSet())
  {
    Serial << F("> module has been reset") << endl;
    modconfig.clearResetFlag();
  }

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);

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
///  setup Module - runs once at power on called from setup()
//

void setupModule()
{
  unsigned int nodeNum = modconfig.nodeNum;
  // configure the module switches, active low
  for (byte i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].attach(SWITCH[i], INPUT_PULLUP);
    moduleSwitch[i].interval(5);
    switchState[i] = false;
  }

  // configure the module LEDs
  for (byte i = 0; i < NUM_LEDS; i++) {
    moduleLED[i].setPin(LED[i]);
  }

  Serial << "> Module has " << NUM_LEDS << " LEDs and " << NUM_SWITCHES << " switches." << endl;
}


void setup()
{
  Serial.begin(115200);
  Serial << endl << F("> ** VLCB 4 in 4 out v1 ** ") << __FILE__ << endl;

  setupVLCB();
  setupModule();

  // end of setup
  DEBUG_PRINT(F("> ready"));
}


void loop()
{
  // do VLCB message, switch and LED processing
  controller.process();

  // process console commands
  processSerialInput();

  // Run the LED code
  for (int i = 0; i < NUM_LEDS; i++) {
    moduleLED[i].run();
  }

  // test for switch input
  processSwitches();

  //  processStartOfDay();
}

void processSwitches(void) {
  bool isSuccess = true;
  for (byte i = 0; i < NUM_SWITCHES; i++) {
    moduleSwitch[i].update();
    if (moduleSwitch[i].changed()) {      
      byte nv = i+1;
      byte nvval = modconfig.readNV(nv);
      bool state;

      //DEBUG_PRINT(F("> Button ") << i << F(" state change detected"));
      //DEBUG_PRINT(F(" NV Value = ") << nvval);

      switch (nvval) {
        case 0:
          // ON and OFF
          state = (moduleSwitch[i].fell());
          //DEBUG_PRINT(F("> Button ") << i << (moduleSwitch[i].fell() ? F(" pressed, send state: ") : F(" released, send state: ")) << state);
          epService.sendEvent(state, i);
          break;

        case 1:
          // Only ON
          if (moduleSwitch[i].fell()) {
            state = true;
            //DEBUG_PRINT(F("> Button ") << i << F(" pressed, send state: ") << state);
            epService.sendEvent(state, i);
          }
          break;

        case 2:
          // Only OFF
          if (moduleSwitch[i].fell()) {
            state = false;
            //DEBUG_PRINT(F("> Button ") << i << F(" pressed, send state: ") << state);
            epService.sendEvent(state, i);
          }
          break;

        case 3:
          // Toggle button
          if (moduleSwitch[i].fell()) {
            switchState[i] = !switchState[i];
            state = (switchState[i]);
            //DEBUG_PRINT(F("> Button ") << i << (moduleSwitch[i].fell() ? F(" pressed, send state: ") : F(" released, send state: ")) << state);
            epService.sendEvent(state, i);
          }
          break;

        default:
          DEBUG_PRINT(F("> Invalid EV value."));
          break;
      }
    } 
  }
}

//
/// called from the VLCB library when a learned event is received
//
void eventhandler(byte index, VLCB::CANFrame *msg)//, bool ison, byte evval)
{
  byte opc = msg->data[0];

  DEBUG_PRINT(F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]));

  unsigned int node_number = (msg->data[1] << 8) + msg->data[2];
  unsigned int event_number = (msg->data[3] << 8) + msg->data[4];
  DEBUG_PRINT(F("> NN = ") << node_number << F(", EN = ") << event_number);
  DEBUG_PRINT(F("> op_code = ") << opc);

  switch (opc) {
    case OPC_ACON:
    case OPC_ASON:
    DEBUG_PRINT(F("> case is opCode ON"));
      for (byte i = 0; i < NUM_LEDS; i++) {
        byte ev = i + 1;
        byte evval = modconfig.getEventEVval(index, ev);
        DEBUG_PRINT(F("> EV = ") << ev << (" Value = ") << evval);

        switch (evval) {
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
    DEBUG_PRINT(F("> case is opCode OFF"));
      for (byte i = 0; i < NUM_LEDS; i++) {
        byte ev = i + 1;
        byte evval = modconfig.getEventEVval(index, ev);

        if (evval > 0) {
          moduleLED[i].off();
        }
      }
      break;
  }
}

void printConfig(void) {
  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Martin Da Costa (MERG M6223) 2023") << endl;
}

//
/// command interpreter for serial console input
//

void processSerialInput()
{
  byte uev = 0;
  char msgstr[32], dstr[32];

  if (Serial.available())
  {
    char c = Serial.read();

    switch (c)
    {
      case 'n':
        // node config
        printConfig();

        // node identity
        Serial << F("> VLCB node configuration") << endl;
        Serial << F("> mode = ") << (modconfig.currentMode == MODE_NORMAL ? "Normal" : "Unitialised") << F(", CANID = ") << modconfig.CANID << F(", node number = ") << modconfig.nodeNum << endl;
        Serial << endl;
        break;

      case 'e':
        // EEPROM learned event data table
        Serial << F("> stored events ") << endl;
        Serial << F("  max events = ") << modconfig.EE_MAX_EVENTS << F(" EVs per event = ") << modconfig.EE_NUM_EVS << F(" bytes per event = ") << modconfig.EE_BYTES_PER_EVENT << endl;

        for (byte j = 0; j < modconfig.EE_MAX_EVENTS; j++)
        {
          if (modconfig.getEvTableEntry(j) != 0)
          {
            ++uev;
          }
        }

        Serial << F("  stored events = ") << uev << F(", free = ") << (modconfig.EE_MAX_EVENTS - uev) << endl;
        Serial << F("  using ") << (uev * modconfig.EE_BYTES_PER_EVENT) << F(" of ") << (modconfig.EE_MAX_EVENTS * modconfig.EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;

        Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

        for (byte j = 0; j < (modconfig.EE_NUM_EVS); j++)
        {
          sprintf(dstr, "EV%03d | ", j + 1);
          Serial << dstr;
        }

        Serial << F("Hash |") << endl;
        Serial << F(" --------------------------------------------------------------") << endl;

        // for each event data line
        for (byte j = 0; j < modconfig.EE_MAX_EVENTS; j++)
        {
          if (modconfig.getEvTableEntry(j) != 0)
          {
            sprintf(dstr, "  %03d  | ", j);
            Serial << dstr;

            // for each data byte of this event
            byte evarray[4];
            modconfig.readEvent(j, evarray);
            for (byte e = 0; e < 4; e++)
            {
              sprintf(dstr, " 0x%02hx | ", evarray[e]);
              Serial << dstr;
            }
            for (byte ev = 1; ev <= modconfig.EE_NUM_EVS; ev++)
            {
              sprintf(dstr, " 0x%02hx | ", modconfig.getEventEVval(j, ev));
              Serial << dstr;
            }

            sprintf(dstr, "%4d |", modconfig.getEvTableEntry(j));
            Serial << dstr << endl;
          }
        }

        Serial << endl;

        break;

      // NVs
      case 'v':
        // note NVs number from 1, not 0
        Serial << "> Node variables" << endl;
        Serial << F("   NV   Val") << endl;
        Serial << F("  --------------------") << endl;

        for (byte j = 1; j <= modconfig.EE_NUM_NVS; j++)
        {
          byte v = modconfig.readNV(j);
          sprintf(msgstr, " - %02d : %3hd | 0x%02hx", j, v, v);
          Serial << msgstr << endl;
        }

        Serial << endl << endl;

        break;

      // CAN bus status
      case 'c':
        can2515.printStatus();
        break;

      case 'h':
        // event hash table
        modconfig.printEvHashTable(false);
        break;

      case 'y':
        // reset CAN bus and VLCB message processing
        can2515.reset();
        break;

      case '*':
        // reboot
        modconfig.reboot();
        break;

      case 'm':
        // free memory
        Serial << F("> free SRAM = ") << modconfig.freeSRAM() << F(" bytes") << endl;
        break;

      case '\r':
        // request setup module
        
        Serial << endl;
        break;
        
       case 'z':
        // Reset module, clear EEPROM
        static bool ResetRq = false;
        static unsigned long ResWaitTime;
        if (!ResetRq) {
          // start timeout timer
          Serial << F(">Reset & EEPROM wipe requested. Press 'z' again within 2 secs to confirm") << endl;
          ResWaitTime = millis();
          ResetRq = true;
        } else {
          // This is a confirmed request
          // 2 sec timeout
          if (ResetRq && ((millis() - ResWaitTime) > 2000)) {
            Serial << F(">timeout expired, reset not performed") << endl;
            ResetRq = false;
          } else {
            //Request confirmed within timeout
            Serial << F(">RESETTING AND WIPING EEPROM") << endl;
            modconfig.resetModule();
            ResetRq = false;
          }
        }
        break;

      default:
        // Serial << F("> unknown command ") << c << endl;
        break;
    }
  }
}
