
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

// globals
ACAN2515 *can;    // CAN bus controller specific object - for MCP2515/25625

//
/// constructor
//

CBUS2515::CBUS2515() {
  _num_rx_buffers = NUM_RX_BUFFS;
  _num_tx_buffers = NUM_TX_BUFFS;
  eventhandler = NULL;
  eventhandlerex = NULL;
  framehandler = NULL;
  _csPin = MCP2515_CS;
  _intPin = MCP2515_INT;
  _osc_freq = OSCFREQ;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
/// default poll arg is set to false, so as not to break existing code
//

bool CBUS2515::begin(bool poll, SPIClass spi) {

  uint16_t ret;
  bool retval = false;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;
  _poll = poll;

  ACAN2515Settings settings(_osc_freq, CANBITRATE);

  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = _num_rx_buffers;
  settings.mTransmitBuffer0Size = _num_tx_buffers;
  settings.mTransmitBuffer1Size = 0;
  settings.mTransmitBuffer2Size = 0;

  // start SPI
  spi.begin();

  // instantiate CAN bus object
  // if in polling mode, the interrupt pin and ISR not used
  if (_poll) {
    can = new ACAN2515(_csPin, spi, 255);
    ret = can->begin(settings, NULL);
  } else {
    can = new ACAN2515(_csPin, spi, _intPin);
    ret = can->begin(settings, [] {can->isr();});
  }

  if (ret == 0) {
    // Serial << F("> CAN controller initialised ok") << endl;
    // save pointer to CAN object so the user can access other parts of the library API
    canp = can;
    retval = true;
  } else {
    // Serial << F("> error initialising CAN controller, error code = ") << ret << endl;
  }

  return retval;
}

//
/// check for unprocessed messages in the buffer
//

bool CBUS2515::available(void) {

  if (_poll) {            // not using interrupts, so poll the interrupt register
    canp->poll();
  }

  return (canp->available());
}

//
/// get next unprocessed message from the buffer
/// must call available first to ensure there is something to get
//

CANFrame CBUS2515::getNextMessage(void) {

  CANMessage message;       // ACAN2515 frame class

  canp->receive(message);

  _msg.id = message.id;
  _msg.len = message.len;
  _msg.rtr = message.rtr;
  _msg.ext = message.ext;
  memcpy(_msg.data, message.data, message.len);

  ++_numMsgsRcvd;
  return _msg;
}

//
/// send a CBUS message
//

bool CBUS2515::sendMessage(CANFrame *msg, bool rtr, bool ext, byte priority) {

  // caller must populate the message data
  // this method will create the correct frame header (CAN ID and priority bits)
  // rtr and ext default to false unless arguments are supplied - see method definition in .h
  // priority defaults to 1011 low/medium

  CANMessage message;       // ACAN2515 frame class
  bool ret = false;

  makeHeader(msg, priority);                      // default priority unless user overrides
  message.id = msg->id;
  message.len = msg->len;
  message.rtr = rtr;
  message.ext = ext;
  memcpy(message.data, msg->data, msg->len);

  ret = canp->tryToSend(message);
  _numMsgsSent += ret;
  return ret;
}

//
/// display the CAN bus status instrumentation
//

void CBUS2515::printStatus(void) {

  Serial << F("> CBUS status:");
  Serial << F(" messages received = ") << _numMsgsRcvd << F(", sent = ") << _numMsgsSent << F(", receive errors = ") << \
         canp->receiveErrorCounter() << F(", transmit errors = ") << canp->transmitErrorCounter() << F(", error flag = ") \
         << canp->errorFlagRegister() << endl;
  return;
}

//
/// reset the MCP2515 transceiver
//

void CBUS2515::reset(void) {
  canp->end();
  delete can;
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
/// this can be tuned according to bus load and available memory
//

void CBUS2515::setNumBuffers(byte num_rx_buffers, byte num_tx_buffers) {
  _num_rx_buffers = num_rx_buffers;
  _num_tx_buffers = num_tx_buffers;
}

//
/// set the MCP2515 crystal frequency
/// default is 16MHz but some modules have an 8MHz crystal
//

void CBUS2515::setOscFreq(unsigned long freq) {
  _osc_freq = freq;
}
