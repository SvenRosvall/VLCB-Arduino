
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
#include <CBUS.h>                   // CBUS class
#include <CBUS2515.h>               // CAN controller
#include <CBUSconfig.h>             // module configuration
#include <CBUSParams.h>             // CBUS parameters
#include <cbusdefs.h>               // MERG CBUS constants
#include <LEDUserInterface.h>

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MODULE_ID = 97;          // CBUS module type

const byte LED_GRN = 4;             // CBUS green SLiM LED pin
const byte LED_YLW = 7;             // CBUS yellow FLiM LED pin
const byte SWITCH0 = 8;             // CBUS push button switch pin

// CBUS objects
CBUSConfig modconfig;               // configuration object
CBUS cbus(&modconfig);              // CBUS object
CBUS2515 cbus2515(&cbus);                  // CAN transport object
LEDUserInterface userInterface(LED_GRN, LED_YLW, SWITCH0);
CBUSLongMessage lmsg(&cbus);        // CBUS RFC0005 long message object

// module name, must be 7 characters, space padded.
unsigned char mname[7] = { 'L', 'M', 'S', 'G', 'E', 'X', ' ' };

// forward function declarations
void eventhandler(byte, CANFrame *);
void framehandler(CANFrame *);
void processSerialInput(void);
void printConfig(void);
void longmessagehandler(void *, const unsigned int, const byte, const byte);

// long message variables
byte streams[] = {1, 2, 3};         // streams to subscribe to
char lmsg_out[32], lmsg_in[32];     // message buffers

//
/// setup CBUS - runs once at power on from setup()
//
void setupCBUS() {

  // set config layout parameters
  modconfig.EE_NVS_START = 10;
  modconfig.EE_NUM_NVS = 10;
  modconfig.EE_EVENTS_START = 50;
  modconfig.EE_MAX_EVENTS = 32;
  modconfig.EE_NUM_EVS = 1;
  modconfig.EE_BYTES_PER_EVENT = (modconfig.EE_NUM_EVS + 4);

  // initialise and load configuration
  modconfig.setEEPROMtype(EEPROM_INTERNAL);
  modconfig.begin();

  Serial << F("> mode = ") << ((modconfig.FLiM) ? "FLiM" : "SLiM") << F(", CANID = ") << modconfig.CANID;
  Serial << F(", NN = ") << modconfig.nodeNum << endl;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  CBUSParams params(modconfig);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setModuleId(MODULE_ID);
  params.setFlags(PF_FLiM | PF_COMBI);

  // assign to CBUS
  cbus.setParams(params.getParams());
  cbus.setName(mname);

  // set CBUS LED pins and assign to CBUS
  cbus.setUI(&userInterface);

  // module reset - if switch is depressed at startup and module is in SLiM mode
  if (userInterface.isButtonPressed() && !modconfig.FLiM) {
    Serial << F("> switch was pressed at startup in SLiM mode") << endl;
    modconfig.resetModule(&userInterface);
  }

  // register our CBUS event handler, to receive event messages of learned events
  cbus.setEventHandler(eventhandler);

  // register our CAN frame handler, to receive *every* CAN frame
  cbus.setFrameHandler(framehandler);

  // subscribe to long message streams and register our handler function
  lmsg.subscribe(streams, sizeof(streams) / sizeof(byte), lmsg_in, 32, longmessagehandler);

  // set CBUS LEDs to indicate the current mode
  cbus.indicateMode(modconfig.FLiM);

  // configure and start CAN bus and CBUS message processing
  cbus2515.setNumBuffers(4, 2);         // more buffers = more memory used, fewer = less
  cbus2515.setOscFreq(16000000UL);      // select the crystal frequency of the CAN module
  cbus2515.setPins(10, 2);              // select pins for CAN bus CE and interrupt connections
  cbus2515.begin();
}

//
/// setup - runs once at power on
//

void setup() {

  Serial.begin (115200);
  Serial << endl << endl << F("> ** CBUS Arduino long message example module ** ") << __FILE__ << endl;

  setupCBUS();

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

  cbus.process();

  //
  /// do RFC0005 CBUS long message processing
  //

  if (!lmsg.process()) {
    Serial << F("> error in long message processing") << endl;
  }

  //
  /// process console commands
  //

  processSerialInput();

  //
  /// check CAN message buffers
  //

  if (cbus2515.canp->receiveBufferPeakCount() > cbus2515.canp->receiveBufferSize()) {
    Serial << F("> receive buffer overflow") << endl;
  }

  if (cbus2515.canp->transmitBufferPeakCount(0) > cbus2515.canp->transmitBufferSize(0)) {
    Serial << F("> transmit buffer overflow") << endl;
  }

  //
  /// check CAN bus state
  //

  byte err;

  if ((err = cbus2515.canp->errorFlagRegister()) != 0) {
    Serial << F("> error flag register = ") << err << endl;
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

  // as an example, display the opcode and the first EV of this event

  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;
  Serial << F("> EV1 = ") << modconfig.getEventEVval(index, 1) << endl;
  return;
}

//
/// user-defined frame processing function
/// called from the CBUS library for *every* CAN frame received
/// it receives a pointer to the received CAN frame
//

void framehandler(CANFrame *msg) {

  // as an example, format and display the received frame

  Serial << "[ " << (msg->id & 0x7f) << "] [" << msg->len << "] [";

  for (byte d = 0; d < msg->len; d++) {
    Serial << " 0x" << _HEX(msg->data[d]);
  }

  Serial << " ]" << endl;
  return;
}

//
/// long message receive handler function
/// called once the user buffer is full, or the message has been completely received
//

void longmessagehandler(void *fragment, const unsigned int fragment_len, const byte stream_id, const byte status) {

  // display the message
  // this will be the complete message if shorter than the provided buffer, or the final fragment if longer

  if (status == CBUS_LONG_MESSAGE_COMPLETE) {
    Serial << F("> received long message, stream = ") << stream_id << F(", len = ") << fragment_len << F(", msg = |") << (char *) fragment << F("|") << endl;
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
      Serial << F("> mode = ") << (modconfig.FLiM ? "FLiM" : "SLiM") << F(", CANID = ") << modconfig.CANID << F(", node number = ") << modconfig.nodeNum << endl;
      Serial << endl;
      break;

    case 'e':

      // EEPROM learned event data table
      Serial << F("> stored events ") << endl;
      Serial << F("  max events = ") << modconfig.EE_MAX_EVENTS << F(" EVs per event = ") << modconfig.EE_NUM_EVS << F(" bytes per event = ") << modconfig.EE_BYTES_PER_EVENT << endl;

      for (byte j = 0; j < modconfig.EE_MAX_EVENTS; j++) {
        if (modconfig.getEvTableEntry(j) != 0) {
          ++uev;
        }
      }

      Serial << F("  stored events = ") << uev << F(", free = ") << (modconfig.EE_MAX_EVENTS - uev) << endl;
      Serial << F("  using ") << (uev * modconfig.EE_BYTES_PER_EVENT) << F(" of ") << (modconfig.EE_MAX_EVENTS * modconfig.EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;

      Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

      for (byte j = 0; j < (modconfig.EE_NUM_EVS); j++) {
        sprintf(dstr, "EV%03d | ", j + 1);
        Serial << dstr;
      }

      Serial << F("Hash |") << endl;

      Serial << F(" --------------------------------------------------------------") << endl;

      // for each event data line
      for (byte j = 0; j < modconfig.EE_MAX_EVENTS; j++) {

        if (modconfig.getEvTableEntry(j) != 0) {
          sprintf(dstr, "  %03d  | ", j);
          Serial << dstr;

          // for each data byte of this event
          byte evarray[4];
          modconfig.readEvent(j, evarray);
          for (byte e = 0; e < 4; e++) {
            sprintf(dstr, " 0x%02hx | ", evarray[e]);
            Serial << dstr;
          }
          for (byte ev = 1; ev <= modconfig.EE_NUM_EVS; ev++) {
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

      for (byte j = 1; j <= modconfig.EE_NUM_NVS; j++) {
        byte v = modconfig.readNV(j);
        sprintf(msgstr, " - %02d : %3hd | 0x%02hx", j, v, v);
        Serial << msgstr << endl;
      }

      Serial << endl << endl;

      break;

    // CAN bus status
    case 'c':

      cbus2515.printStatus();
      break;

    case 'h':
      // event hash table
      modconfig.printEvHashTable(false);
      break;

    case 'y':
      // reset CAN bus and CBUS message processing
      cbus2515.reset();
      break;

    case '*':
      // reboot
      modconfig.reboot();
      break;

    case 'm':
      // free memory
      Serial << F("> free SRAM = ") << modconfig.freeSRAM() << F(" bytes") << endl;
      break;

    case 'l':
      // send a long message with stream ID = 2
      uev = Serial.readBytesUntil('\n', lmsg_out, 32);
      lmsg.sendLongMessage(lmsg_out, uev, 2);
      break;

    case 'd':
      Serial << F("> tx buffer = ") << cbus2515.canp->transmitBufferSize(0) << F(", ") <<  cbus2515.canp->transmitBufferCount(0) << F(", ") << cbus2515.canp->transmitBufferPeakCount(0) << endl;
      Serial << F("> rx buffer = ") << cbus2515.canp->receiveBufferSize() << F(", ") << cbus2515.canp->receiveBufferCount() << F(", ") << cbus2515.canp->receiveBufferPeakCount() << endl;
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
