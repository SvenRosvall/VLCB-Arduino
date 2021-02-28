
//
///
//

/*
  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

/*
      3rd party libraries needed for compilation: (not for binary-only distributions)

      Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
*/

// 3rd party libraries
#include <Streaming.h>

// CBUS library header files
#include <CBUS2515.h>               // CAN controller and CBUS class
#include <CBUSswitch.h>             // pushbutton switch
#include <CBUSLED.h>                // CBUS LEDs
#include <CBUSconfig.h>             // module configuration
#include <cbusdefs.h>               // MERG CBUS constants

// local header
#include "defs.h"

// CBUS objects
CBUS2515 CBUS;                      // CBUS object
CBUSConfig config;                  // configuration object
CBUSLED ledGrn, ledYlw;             // two LED objects
CBUSSwitch pb_switch;               // switch object

// module objects
CBUSSwitch moduleSwitch;            // an example switch as input
CBUSLED moduleLED;                  // an example LED as output

// CBUS module parameters
unsigned char params[21];

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { '1', 'I', 'N', '1', 'O', 'U', 'T' };

// forward function declarations
void eventhandler(byte index, byte opc);

//
/// setup - runs once at power on
//

void setup() {

  Serial.begin (115200);
  Serial << endl << endl << F("> ** CBUS empty module v1 ** ") << __FILE__ << endl;

  // set config layout parameters
  config.EE_NVS_START = 10;
  config.EE_NUM_NVS = 10;
  config.EE_EVENTS_START = 50;
  config.EE_MAX_EVENTS = 64;
  config.EE_NUM_EVS = 1;
  config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

  // initialise and load configuration
  config.setEEPROMtype(EEPROM_INTERNAL);
  config.begin();

  Serial << F("> mode = ") << ((config.FLiM) ? "FLiM" : "SLiM") << F(", CANID = ") << config.CANID;
  Serial << F(", NN = ") << config.nodeNum << endl;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  params[0] = 20;                     //  0 num params = 10
  params[1] = 0xa5;                   //  1 manf = MERG, 165
  params[2] = VER_MIN;                //  2 code minor version
  params[3] = MODULE_ID;              //  3 module id, 99 = undefined
  params[4] = config.EE_MAX_EVENTS;   //  4 num events
  params[5] = config.EE_NUM_EVS;      //  5 num evs per event
  params[6] = config.EE_NUM_NVS;      //  6 num NVs
  params[7] = VER_MAJ;                //  7 code major version
  params[8] = 0x07;                   //  8 flags = 7, FLiM, consumer/producer
  params[9] = 0x32;                   //  9 processor id = 50
  params[10] = PB_CAN;                // 10 interface protocol = CAN, 1
  params[11] = 0x00;
  params[12] = 0x00;
  params[13] = 0x00;
  params[14] = 0x00;
  params[15] = '3';
  params[16] = '2';
  params[17] = '8';
  params[18] = 'P';
  params[19] = CPUM_ATMEL;
  params[20] = VER_BETA;

  // assign to CBUS
  CBUS.setParams(params);
  CBUS.setName(mname);

  // set LED and switch pins and assign to CBUS
  ledGrn.setPin(LED_GRN);
  ledYlw.setPin(LED_YLW);
  CBUS.setLEDs(ledGrn, ledYlw);

  // initialise CBUS switch
  pb_switch.setPin(SWITCH0, LOW);
  CBUS.setSwitch(pb_switch);

  // module reset - if switch is depressed at startup and module is in SLiM mode
  pb_switch.run();

  if (pb_switch.isPressed() && !config.FLiM) {
    Serial << F("> switch was pressed at startup in SLiM mode") << endl;
    config.resetModule(ledGrn, ledYlw, pb_switch);
  }

  // opportunity to set default NVs after module reset
  if (EEPROM.read(5) == 99) {
    Serial << F("> module has been reset") << endl;
    EEPROM.write(5, 0);
  }

  // register our CBUS event handler, to receive event messages of learned events
  CBUS.setEventHandler(eventhandler);

  // set CBUS LEDs to indicate mode
  CBUS.indicateMode(config.FLiM);

  // configure and start CAN bus and CBUS message processing
  CBUS.setNumBuffers(2);         // more buffers = more memory used, fewer = less
  CBUS.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module
  CBUS.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections
  CBUS.begin();

  // configure the module switch, attached to pin 7, active low
  moduleSwitch.setPin(7, LOW);

  // configure the module LED, attached to pin 8 via a 1K resistor
  moduleLED.setPin(8);

  // end of setup
  Serial << F("> ready") << endl << endl;
}

//
/// loop - runs forever
//

void loop() {

  //
  /// do CBUS message, switch and LED processing
  //

  CBUS.process();

  //
  /// process console commands
  //

  processSerialInput();

  //
  /// give the switch and LED code some time to run
  //

  moduleSwitch.run();
  moduleLED.run();

  //
  /// test for switch input
  /// as an example, it must be have been pressed or released for at least half a second
  /// then send a long CBUS event with opcode ACON for on and ACOF for off
  /// event number (EN) is 1

  /// you can just watch for this event in FCU or JMRI, or teach it to another CBUS consumer module
  //

  if (moduleSwitch.stateChanged()) {

    CANFrame msg;
    msg.id = config.CANID;
    msg.len = 5;
    msg.data[0] = (moduleSwitch.isPressed() ? OPC_ACON : OPC_ACOF);
    msg.data[1] = highByte(config.nodeNum);
    msg.data[2] = lowByte(config.nodeNum);
    msg.data[3] = 0;
    msg.data[4] = 1;            // event number (EN) = 1

    if (CBUS.sendMessage(&msg)) {
      Serial << F("> sent CBUS message") << endl;
    } else {
      Serial << F("> error sending CBUS message") << endl;
    }
  }

  //
  /// bottom of loop()
  //
}

//
/// user-defined event processing function
/// called from the CBUS library when a learned event is received
/// it receives the event table index and the CAN frame
//

void eventhandler(byte index, CANFrame *msg) {

  // as an example, control an LED if the first EV equals 1

  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;

  // read the value of the first event variable (EV) associated with this learned event
  byte ev1 = config.getEventEVval(index, 1);
  Serial << F("> EV1 = ") << evval << endl;

  // set the LED according to the opcode of the received event, if the first EV equals 1
  // we use the blink() method of the LED object as an example

  if (evval == 1) {
    if (msg->data[0] == OPC_ACON) {
      Serial << F("> switching the LED on") << endl;
      moduleLED.blink();
    } else if (msg->data[0] == OPC_ACOF) {
      Serial << F("> switching the LED off") << endl;
      moduleLED.off();
    }
  }

  return;
}

//
/// print code version config details and copyright notice
//

void printConfig(void) {

  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Duncan Greenwood (MERG M5767) 2019") << endl;
  return;
}

//
/// command interpreter for serial console input
//

void processSerialInput(void) {

  byte uev = 0;
  char msgstr[32], dstr[32];

  if (Serial.available()) {

    char c = Serial.read();

    switch (c) {

      case 'n':

        // node config
        printConfig();

        // node identity
        Serial << F("> CBUS node configuration") << endl;
        Serial << F("> mode = ") << (config.FLiM ? "FLiM" : "SLiM") << F(", CANID = ") << config.CANID << F(", node number = ") << config.nodeNum << endl;
        Serial << endl;
        break;

      case 'e':

        // EEPROM learned event data table
        Serial << F("> stored events ") << endl;

        sprintf(msgstr, "  max events = %d, EVs per event = %d, bytes per event = %d", config.EE_MAX_EVENTS, config.EE_NUM_EVS, config.EE_BYTES_PER_EVENT);
        Serial << msgstr << endl;

        for (byte j = 0; j < config.EE_MAX_EVENTS; j++) {
          if (config.getEvTableEntry(j) != 0) {
            ++uev;
          }
        }

        Serial << F("  stored events = ") << uev << F(", free = ") << (config.EE_MAX_EVENTS - uev) << endl;
        Serial << F("  using ") << (uev * config.EE_BYTES_PER_EVENT) << F(" of ") << (config.EE_MAX_EVENTS * config.EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;

        Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

        for (byte j = 0; j < (config.EE_NUM_EVS); j++) {
          sprintf(dstr, "EV%03d | ", j + 1);
          Serial << dstr;
        }

        Serial << F("Hash |") << endl;

        Serial << F(" --------------------------------------------------------------") << endl;

        // for each event data line
        for (byte j = 0; j < config.EE_MAX_EVENTS; j++) {

          if (config.getEvTableEntry(j) != 0) {
            sprintf(dstr, "  %03d  | ", j);
            Serial << dstr;

            // for each data byte of this event
            for (byte e = 0; e < (config.EE_NUM_EVS + 4); e++) {
              sprintf(dstr, " 0x%02hx | ", config.readEEPROM(config.EE_EVENTS_START + (j * config.EE_BYTES_PER_EVENT) + e));
              Serial << dstr;
            }

            sprintf(dstr, "%4d |", config.getEvTableEntry(j));
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

        for (byte j = 1; j <= config.EE_NUM_NVS; j++) {
          sprintf(msgstr, " - %02d : %3hd | 0x%02hx", j, config.readNV(j), config.readNV(j));
          Serial << msgstr << endl;
        }

        Serial << endl << endl;

        break;

      // CAN bus status
      case 'c':

        CBUS.printStatus();
        break;

      case 'h':
        // event hash table
        config.printEvHashTable(false);
        break;

      case 'y':
        // reset CAN bus and CBUS message processing
        CBUS.reset();
        break;

      case '*':
        // reboot
        config.reboot();
        break;

      case 'm':
        // free memory
        Serial << F("> free SRAM = ") << config.freeSRAM() << F(" bytes") << endl;
        break;

      case '\r':
      case '\n':
        Serial << endl;
        break;

      default:
        // Serial << F("> unknown command ") << c << endl;
        break;
    }
  }
}
