
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

#include <CBUSLED.h>
#include <CBUSswitch.h>
#include <CBUSconfig.h>
#include <cbusdefs.h>

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

class CANFrame {

  public:

      CANFrame();
      uint32_t id;
      bool ext;
      bool rtr;
      uint8_t len;
      uint8_t data[8];
};

//
/// an abstract class to encapsulate CAN bus and CBUS processing
/// it must be implemented by a derived subclass
//

class CBUS {

  public:
    
    // these methods are pure virtual and must be implemented by the derived class
    // as a consequence, it is not possible to create an instance of this class
    
    virtual bool begin(void) = 0;
    virtual bool available(void) = 0;
    virtual CANFrame getNextMessage(void) = 0;
    virtual bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false) = 0;
    virtual void reset(void) = 0;

    // implementations of these methods are provided in the base class

    bool sendWRACK(void);
    bool sendCMDERR(byte cerrno);
    void CANenumeration(void);
    byte getCANID(unsigned long header);
    bool isExt(CANFrame *msg);
    bool isRTR(CANFrame *msg);
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
    void setEventHandler(void (*fptr)(byte index, CANFrame *msg));
    void setFrameHandler(void (*fptr)(CANFrame *msg));
    void setFrameHandler(void (*fptr)(CANFrame *msg), byte *opcodes, byte num_opcodes);
    void makeHeader(CANFrame *msg);

  protected:
    CANFrame _msg;
    unsigned int _numMsgsSent, _numMsgsRcvd;
    byte _lastErr;
    CBUSLED _ledGrn, _ledYlw;
    CBUSSwitch _sw;
    unsigned char *_mparams;
    unsigned char *_mname;
    void (*eventhandler)(byte index, CANFrame *msg);
    void (*framehandler)(CANFrame *msg);
    byte *_opcodes;
    byte _num_opcodes;
    bool bModeChanging, bCANenum, bLearn, bCANenumComplete;
    unsigned long timeOutTimer, CANenumTime;
    char msgstr[64], dstr[64];
    byte enums = 0, selected_id;
    byte enum_responses[16];               // 128 bits for storing CAN ID enumeration results
    bool enumeration_required;

};
