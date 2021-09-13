
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

#pragma once

// header files

#include <CBUS.h>               // abstract base class
#include <CBUS2515.h>           // header for this class
#include <ACAN2515.h>           // ACAN2515 library
#include <SPI.h>

// constants

static const byte MCP2515_CS = 10;                          // SPI chip select pin
static const byte MCP2515_INT = 2;                          // interrupt pin
static const byte NUM_RX_BUFFS = 4;                         // default value
static const byte NUM_TX_BUFFS = 2;                         // default value
static const uint32_t CANBITRATE = 125000UL;                // 125Kb/s - fixed for CBUS
static const uint32_t OSCFREQ = 16000000UL;                 // crystal frequency default

//
/// an implementation of the absract base CBUS class
/// to support the MCP2515/25625 CAN controllers
//

class CBUS2515 : public CBUSbase {

public:

  CBUS2515();

  // these methods are declared virtual in the base class and must be implemented by the derived class
  bool begin(bool poll = false, SPIClass spi = SPI);    // note default args
  bool available(void);
  CANFrame getNextMessage(void);
  bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY);    // note default arguments
  void reset(void);

  // these methods are specific to this implementation
  // they are not declared or implemented by the base CBUS class
  void setNumBuffers(byte num_rx_buffers, byte num_tx_buffers = 0);      // note default arg
  void setPins(byte CSpin, byte intPin);
  void printStatus(void);
  void setOscFreq(unsigned long freq);

  ACAN2515 *canp;   // pointer to CAN object so user code can access its members

private:
  unsigned long _osc_freq;
  byte _csPin, _intPin;
  byte _num_rx_buffers, _num_tx_buffers;
  bool _poll;
};
