// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

// header files

#include <Controller.h>
#include <CanTransport.h>
#include <ACAN2515.h>           // ACAN2515 library
#include <SPI.h>

namespace VLCB
{

// constants

static const byte MCP2515_CS = 10;                          // SPI chip select pin
static const byte MCP2515_INT = 2;                          // interrupt pin
static const byte NUM_RX_BUFFS = 4;                         // default value
static const byte NUM_TX_BUFFS = 2;                         // default value
static const uint32_t CANBITRATE = 125000UL;                // 125Kb/s - fixed for VLCB
static const uint32_t OSCFREQ = 16000000UL;                 // crystal frequency default

//
/// an implementation of the Transport interface class
/// to support the MCP2515/25625 CAN controllers
//

class CAN2515 : public CanTransport
{
public:

  CAN2515();

  // these methods are declared virtual in the base class and must be implemented by the derived class
#ifdef ARDUINO_ARCH_RP2040
  bool begin(bool poll = false, SPIClassRP2040 & spi = SPI);    // note default args
#else
  bool begin(bool poll = false, SPIClass & spi = SPI);
#endif
  bool available() override;
  CANFrame getNextCanFrame() override;
  bool sendCanFrame(CANFrame *frame) override;
  void reset() override;

  // these methods are specific to this implementation
  // they are not declared or implemented by the Transport interface class
  void setNumBuffers(byte num_rx_buffers, byte num_tx_buffers = 0);      // note default arg
  void printStatus();
  void setOscFreq(unsigned long freq);

#ifdef ARDUINO_ARCH_RP2040
  void setPins(byte cs_pin, byte int_pin, byte mosi_pin, byte miso_pin, byte sck_pin);
#else
  void setPins(byte cs_pin, byte int_pin);
#endif

  virtual unsigned int receiveCounter()override { return _numMsgsRcvd; }
  virtual unsigned int transmitCounter()override { return _numMsgsRcvd; }
  virtual unsigned int receiveErrorCounter()override { return canp->receiveErrorCounter(); }
  virtual unsigned int transmitErrorCounter()override { return canp->transmitErrorCounter(); }
  virtual unsigned int errorStatus()override { return canp->errorFlagRegister(); }

  ACAN2515 *canp;   // pointer to CAN object so user code can access its members

private:
  unsigned int _numMsgsSent, _numMsgsRcvd;
  unsigned long _osc_freq;
  byte _csPin, _intPin;
  byte _num_rx_buffers, _num_tx_buffers;
  bool _poll;

#ifdef ARDUINO_ARCH_RP2040
  byte _mosi_pin, _miso_pin, _sck_pin;
#endif
};

}