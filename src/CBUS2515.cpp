
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

// Arduino libraries
#include <SPI.h>

// 3rd party libraries
#include <Streaming.h>

// CBUS MCP2515 library
#include <CBUS2515.h>

// CAN bus controller specific object - for MCP2515/25625
ACAN2515 *can;

// CBUS configuration object, declared externally
extern CBUSConfig config;

//
/// constructor
//

CBUS2515::CBUS2515() {
  numbuffers = NUM_RECV_BUFFS;
  eventhandler = NULL;
  _csPin = 10;
  _intPin = 2;
  _osc_freq = 16000000UL;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
//

bool CBUS2515::begin(void) {

  uint16_t ret;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;

  ACAN2515Settings settings(_osc_freq, canbitrate);

  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = numbuffers;
  settings.mTransmitBuffer0Size = 0;
  settings.mTransmitBuffer1Size = 0;
  settings.mTransmitBuffer2Size = 0;

  // start SPI
  SPI.begin();

  // instantiate CAN bus object
  can = new ACAN2515(_csPin, SPI, _intPin);

  // Serial << F("> initialising CAN controller") << endl;
  ret = can->begin(settings, [] {can->isr();});

  if (ret == 0) {
    // Serial << F("> CAN controller initialised ok") << endl;
    return true;
  } else {
    Serial << F("> error initialising CAN controller, error code = ") << ret << endl;
    return false;
  }

}

//
/// check for unprocessed messages in the buffer
//

bool CBUS2515::available(void) {

  return (can->available());
}

//
/// get next unprocessed message from the buffer
//

CANFrame CBUS2515::getNextMessage(void) {

  CANMessage message;
  can->receive(message);

  _msg.id = message.id;
  _msg.len = message.len;
  _msg.rtr = message.rtr;
  _msg.ext = message.ext;
  memcpy(_msg.data, message.data, 8);

  ++_numMsgsRcvd;
  return _msg;
}

//
/// send a CBUS message
//

bool CBUS2515::sendMessage(CANFrame *msg) {

  // caller must populate the frame data
  // header is set by the CANFrame constructor
  
  CANMessage message;
  bool _res;

  message.id = msg->id;
  message.len = msg->len;
  message.rtr = msg->rtr;
  message.ext = msg->ext;
  memcpy(message.data, msg->data, 8);

  _res = can->tryToSend(message);
  _numMsgsSent++;

  return _res;
}

//
/// display the CAN bus status instrumentation
//

void CBUS2515::printStatus(void) {

  Serial << F("> CBUS status: ");
  Serial << F(" messages received = ") << _numMsgsRcvd << F(", sent = ") << _numMsgsSent << F(", receive errors = ") << can->receiveErrorCounter() << \
         F(", transmit errors = ") << can->transmitErrorCounter() << endl;
  return;
}

void CBUS2515::reset(void) {

  can->end();
  begin();
}

//
/// set the CS and interrupt pins - option to override defaults
//

void CBUS2515::setPins(byte csPin, byte intPin) {

  _csPin = csPin;
  _intPin = intPin;
}

//
/// set the number of CAN frame receive buffers
/// this can be tuned according to load and available memory
//

void CBUS2515::setNumBuffers(byte num) {
  numbuffers = num;
}

//
/// set the MCP2515 crystal frequency
/// default is 16MHz but some modules have an 8MHz crystal
//

void CBUS2515::setOscFreq(unsigned long freq) {
  _osc_freq = freq;
}
