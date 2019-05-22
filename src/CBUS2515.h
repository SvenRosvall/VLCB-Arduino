
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

#if !defined __CBUS2515_H__
#define __CBUS2515_H__

// header files

#include <CBUS.h>               // abstract base class
#include <CBUS2515.h>           // header for this class
#include <ACAN2515.h>           // ACAN2515 library
#include <CBUSconfig.h>         // CBUS config class
#include <CBUSLED.h>            // CBUS LED class
#include <CBUSswitch.h>         // CBUS switch class

// constants

static const byte MCP2515_CS = 10;                          // SPI chip select pin
static const byte MCP2515_INT = 2;                          // interrupt pin
static const byte NUM_RECV_BUFFS = 4;                       // default value

static const uint32_t oscfreq = 16UL * 1000UL * 1000UL;     // 16MHz crystal
static const uint32_t canbitrate = 125UL * 1000UL;          // 125Kb/s

//
/// an implementation of the absract base CBUS class
/// to support the MCP2515/25625 CAN controllers
//

class CBUS2515 : public CBUS {

  public:

    CBUS2515();

    // these methods are declared virtual in the base class and must be implemented by the derived class
    bool begin(void);
    bool available(void);
    CANFrame getNextMessage(void);
    bool sendMessage(CANFrame *msg);
    void printStatus(void);
    void reset(void);
    void setNumBuffers(byte num);
    void setPins(byte CSpin, byte intPin);
};

#endif
