
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

#if !defined __CBUS_H__
#define __CBUS_H__

#include <ACAN2515.h>
#include <CBUSLED.h>
#include <CBUSswitch.h>

static const byte MCP2515_CS = 10;                          // SPI chip select pin
static const byte MCP2515_INT = 2;                          // interrupt pin
static const byte NUM_RECV_BUFFS = 4;                       // default value

static const uint32_t oscfreq = 16UL * 1000UL * 1000UL;     // 16MHz crystal
static const uint32_t canbitrate = 125UL * 1000UL;          // 125Kb/s

static const unsigned int SW_TR_HOLD = 6000;       // CBUS push button hold time for SLiM/FLiM transition in millis

//
/// CBUS modes
//

enum {
  MODE_SLIM = 0,
  MODE_FLIM = 1,
  MODE_CHANGING = 2
};

//
/// CAN/CBUS message type
//

typedef struct {
  unsigned long id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8];
} CANFrame;

//
/// forward function declarations
//

unsigned long ascii_pair_to_byte(const char *pair);
char *FrameToGridConnect(const CANFrame *frame, char dest[]);
CANFrame *GridConnectToFrame(const char string[], CANFrame *msg);

//
/// a class to encapsulate CAN bus and CBUS processing
//

class CBUS {

  public:
    CBUS();
    bool begin(void);
    bool available();
    CANFrame getNextMessage();
    bool sendMessage(CANFrame *msg);
    bool sendWRACK(void);
    bool sendCMDERR(byte cerrno);
    void CANenumeration(void);
    byte getCANID(unsigned long header);
    void printStatus(void);
    bool isExt(CANFrame *msg);
    bool isRTR(CANFrame *msg);
    void reset(void);
    void process(void);
    void initFLiM(void);
    void revertSLiM(void);
    void setSLiM(void);
    void renegotiate(void);
    void setLEDs(CBUSLED ledGrn, CBUSLED ledYlw);
    void setSwitch(CBUSSwitch sw);
    void setParams(unsigned char *mparams);
    void setName(unsigned char *mname);
    void checkCANenum(void);
    void indicateMode(byte mode);
    void setNumBuffers(byte num);
    void setPins(byte CSpin, byte intPin);
    void setEventHandler(void (*fptr)(byte index, CANFrame *msg));

  protected:
    CANFrame _msg;
    byte _csPin, _intPin;
    unsigned int _numMsgsSent, _numMsgsRcvd;
    byte _lastErr;
    CBUSLED _ledGrn, _ledYlw;
    CBUSSwitch _sw;
    unsigned char *_mparams;
    unsigned char *_mname;
    byte numbuffers;
    void (*eventhandler)(byte index, CANFrame *msg);

};

#endif
