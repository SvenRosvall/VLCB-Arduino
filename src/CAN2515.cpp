// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// Arduino libraries
#include <SPI.h>

// 3rd party libraries
#include <Streaming.h>

// VLCB MCP2515 library
#include <CAN2515.h>

namespace VLCB
{

//
/// constructor
//
CAN2515::CAN2515()
{
  initMembers();
}

void CAN2515::initMembers()
{
  _num_rx_buffers = NUM_RX_BUFFS;
  _num_tx_buffers = NUM_TX_BUFFS;
  _csPin = MCP2515_CS;
  _intPin = MCP2515_INT;
  _osc_freq = OSCFREQ;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
/// default poll arg is set to false, so as not to break existing code
//
#ifdef ARDUINO_ARCH_RP2040
bool CAN2515::begin(bool poll, SPIClassRP2040 spi)
#else
bool CAN2515::begin(bool poll, SPIClass spi)
#endif
{
  uint16_t ret;
  bool retval = false;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;
  _poll = poll;

  ACAN2515Settings settings(_osc_freq, CANBITRATE);

  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = _num_rx_buffers;
  settings.mTransmitBuffer0Size = _num_tx_buffers;

#if defined ESP8266
  settings.mTransmitBuffer1Size = 1;      // ESP8266 doesn't like new of zero bytes
  settings.mTransmitBuffer2Size = 1;
#else
  settings.mTransmitBuffer1Size = 0;
  settings.mTransmitBuffer2Size = 0;
#endif

#ifdef ARDUINO_ARCH_RP2040
  spi.setTX(_mosi_pin);
  spi.setRX(_miso_pin);
  spi.setSCK(_sck_pin);
  spi.setCS(_csPin);
#endif

  // start SPI
  spi.begin();

  // instantiate CAN bus object
  // if in polling mode, the interrupt pin and ISR not used
  if (_poll)
  {
    canp = new ACAN2515(_csPin, spi, 255);
    ret = canp->begin(settings, NULL);
  }
  else
  {
    canp = new ACAN2515(_csPin, spi, _intPin);
    static ACAN2515 * lcanp; // Need a variable with static storage duration for use in the lambda.
    lcanp = canp;
    ret = canp->begin(settings, [] { lcanp->isr(); });
  }

  if (ret == 0)
  {
    // DEBUG_SERIAL << F("> CAN controller initialised ok") << endl;
    retval = true;
  }
  else
  {
    // DEBUG_SERIAL << F("> error initialising CAN controller, error code = ") << ret << endl;
  }

  return retval;
}

//
/// check for unprocessed messages in the buffer
//
bool CAN2515::available()
{
  if (_poll)
  {            // not using interrupts, so poll the interrupt register
    canp->poll();
  }
  return (canp->available());
}

//
/// get next unprocessed message from the buffer
/// must call available first to ensure there is something to get
//
CANFrame CAN2515::getNextMessage()
{
  // DEBUG_SERIAL << F("CAN2515 trying to get next message.") << endl;
  CANMessage message;       // ACAN2515 frame class

  canp->receive(message);

  CANFrame msg;
  msg.id = message.id;
  msg.len = message.len;
  msg.rtr = message.rtr;
  msg.ext = message.ext;
  memcpy(msg.data, message.data, message.len);
//  DEBUG_SERIAL << F("CAN2515 getNextMessage id=") << (msg.id & 0x7F) << " len=" << msg.len << " rtr=" << msg.rtr;
//  if (msg.len > 0)
//    DEBUG_SERIAL << " op=" << _HEX(msg.data[0]);
//  DEBUG_SERIAL << endl;

  ++_numMsgsRcvd;
  return msg;
}

//
/// send a VLCB message
//
bool CAN2515::sendMessage(CANFrame *msg, bool rtr, bool ext, byte priority)
{
  // caller must populate the message data
  // this method will create the correct frame header (CAN ID and priority bits)
  // rtr and ext default to false unless arguments are supplied - see method definition in .h
  // priority defaults to 1011 low/medium

  CANMessage message;       // ACAN2515 frame class
  makeHeader(msg, priority);                      // default priority unless user overrides
  message.id = msg->id;
  message.len = msg->len;
  message.rtr = rtr;
  message.ext = ext;
  memcpy(message.data, msg->data, msg->len);

//  DEBUG_SERIAL << F("CAN2515 sendMessage id=") << (msg->id & 0x7F) << " len=" << msg->len << " rtr=" << rtr;
//  if (msg->len > 0)
//    DEBUG_SERIAL << " op=" << _HEX(msg->data[0]);
//  DEBUG_SERIAL << endl;

  bool ret = canp->tryToSend(message);
  _numMsgsSent += ret;

  // Simple workaround for sending many messages. Let the underlying hardware some time to send this message before next.
  // TODO: Replace this with monitoring of the transmit queue.
  delay(1);

  return ret;
}

//
/// display the CAN bus status instrumentation
//
void CAN2515::printStatus()
{
  // removed so that no libraries produce serial output
  // can be implemented in user's sketch

  /*
    DEBUG_SERIAL << F("> VLCB status:");
    DEBUG_SERIAL << F(" messages received = ") << _numMsgsRcvd << F(", sent = ") << _numMsgsSent << F(", receive errors = ") << endl;
           // canp->receiveErrorCounter() << F(", transmit errors = ") << canp->transmitErrorCounter() << F(", error flag = ")
           // << canp->errorFlagRegister()
           // << endl;
   */
}

//
/// reset the MCP2515 transceiver
//
void CAN2515::reset()
{
  canp->end();
  delete canp;
  begin();
}

//
/// set the CS and interrupt pins - option to override defaults
//
#ifdef ARDUINO_ARCH_RP2040
void CAN2515::setPins(byte cs_pin, byte int_pin, byte mosi_pin, byte miso_pin, byte sck_pin)
#else
void CAN2515::setPins(byte cs_pin, byte int_pin)
#endif
{

#ifdef ARDUINO_ARCH_RP2040
  _mosi_pin = mosi_pin;
  _miso_pin = miso_pin;
  _sck_pin = sck_pin;
#endif

  _csPin = cs_pin;
  _intPin = int_pin;
}

//
/// set the number of CAN frame receive buffers
/// this can be tuned according to bus load and available memory
//
void CAN2515::setNumBuffers(byte num_rx_buffers, byte num_tx_buffers)
{
  _num_rx_buffers = num_rx_buffers;
  _num_tx_buffers = num_tx_buffers;
}

//
/// set the MCP2515 crystal frequency
/// default is 16MHz but some modules have an 8MHz crystal
//
void CAN2515::setOscFreq(unsigned long freq)
{
  _osc_freq = freq;
}

//
/// actual implementation of the makeHeader method
/// so it can be called directly or as a Controller class method
/// the 11 bit ID of a standard CAN frame is comprised of: (4 bits of CAN priority) + (7 bits of CAN ID)
/// priority = 1011 (0xB hex, 11 dec) as default argument, which translates to medium/low
//
inline void makeHeader_impl(CANFrame *msg, byte id, byte priority)
{
  msg->id = (priority << 7) + (id & 0x7f);
}

//
/// utility method to populate a VLCB message header
//
void CAN2515::makeHeader(CANFrame *msg, byte priority)
{
  makeHeader_impl(msg, controller->getModuleCANID(), priority);
}

}