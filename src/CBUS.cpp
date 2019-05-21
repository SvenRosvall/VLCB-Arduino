
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
#include <ACAN2515.h>

// CBUS libraries
#include <CBUSConfig.h>
#include "CBUS.h"
#include "cbusdefs.h"

// CAN bus controller objects
ACAN2515 *can;
extern CBUSConfig config;

// global variables
bool bModeChanging, bCANenum, bLearn, bCANenumComplete;
unsigned long timeOutTimer, CANenumTime;
char msgstr[64], dstr[64];
byte enums = 0, selected_id;
byte enum_responses[16];               // 128 bits for storing CAN ID enumeration results

//
/// constructor
//

CBUS::CBUS() {
  numbuffers = NUM_RECV_BUFFS;
  eventhandler = NULL;
  _csPin = 10;
  _intPin = 2;
}

//
/// set the CS and interrupt pins - option to override defaults
//

void CBUS::setPins(byte csPin, byte intPin) {

  _csPin = csPin;
  _intPin = intPin;
}

//
/// set the number of CAN frame receive buffers
/// this can be tuned according to load and available memory
//

void CBUS::setNumBuffers(byte num) {
  numbuffers = num;
}

//
/// register the user handler for event opcodes
//

void CBUS::setEventHandler(void (*fptr)(byte index, CANFrame *msg)) {
  eventhandler = fptr;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
//

bool CBUS::begin(void) {

  uint16_t ret;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;

  ACAN2515Settings settings(oscfreq, canbitrate);

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
/// assign a parameter set to the module
//

void CBUS::setParams(unsigned char *mparams) {
  _mparams = mparams;
}

//
/// assign a name to the module
//

void CBUS::setName(unsigned char *mname) {
  _mname = mname;
}

//
/// set module to SLiM mode
//

void CBUS::setSLiM(void) {

  bModeChanging = false;
  config.setNodeNum(0);
  config.setFLiM(false);
  config.setCANID(0);
  indicateMode(config.FLiM);
}

//
/// check for unprocessed messages in the buffer
//

bool CBUS::available(void) {

  return (can->available());
}

//
/// get next unprocessed message from the buffer
//

CANFrame CBUS::getNextMessage(void) {

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

bool CBUS::sendMessage(CANFrame *msg) {

  // caller must populate the frame data
  // we set the CAN ID and priority in the header here

  CANMessage message;
  bool _res;

  message.id = config.CANID;
  message.len = msg->len;
  message.rtr = msg->rtr;
  message.ext = msg->ext;
  memcpy(message.data, msg->data, 8);

  // set the CBUS message priority - zeroes equate to higher priority
  // bits 7 and 8 are the minor priority, so 11 = 'low'
  bitSet(message.id, 7);
  bitSet(message.id, 8);

  // bits 9 and 10 are the major priority, so 01 = 'medium'
  bitClear(message.id, 9);
  bitSet(message.id, 10);

  _res = can->tryToSend(message);
  _numMsgsSent++;

  return _res;
}

//
/// extract CANID from CAN frame header
//

byte CBUS::getCANID(unsigned long header) {

  return (header & 0x7f);
}

//
/// send a WRACK (write acknowledge) message
//

bool CBUS::sendWRACK(void) {

  // send a write acknowledgement response

  memset(&_msg, 0, sizeof(_msg));
  _msg.len = 3;
  _msg.ext = false;
  _msg.rtr = false;
  _msg.data[0] = OPC_WRACK;
  _msg.data[1] = highByte(config.nodeNum);
  _msg.data[2] = lowByte(config.nodeNum);

  return sendMessage(&_msg);
}

//
/// send a CMDERR (command error) message
//

bool CBUS::sendCMDERR(byte cerrno) {

  // send a command error response

  memset(&_msg, 0, sizeof(_msg));
  _msg.len = 4;
  _msg.ext = false;
  _msg.rtr = false;
  _msg.data[0] = OPC_CMDERR;
  _msg.data[1] = highByte(config.nodeNum);
  _msg.data[2] = lowByte(config.nodeNum);
  _msg.data[3] = cerrno;

  return sendMessage(&_msg);
}

//
/// display the CAN bus status instrumentation
//

void CBUS::printStatus(void) {

  Serial << F("> CBUS status: ");
  Serial << F(" messages received = ") << _numMsgsRcvd << F(", sent = ") << _numMsgsSent << F(", receive errors = ") << can->receiveErrorCounter() << \
         F(", transmit errors = ") << can->transmitErrorCounter() << endl;

  return;
}

bool CBUS::isExt(CANFrame *amsg) {

  return (amsg->ext);
}

bool CBUS::isRTR(CANFrame *amsg) {

  return (amsg->rtr);
}

void CBUS::reset(void) {

  can->end();
  begin();
  return;
}

//
/// if in FLiM mode, initiate a CAN ID enumeration cycle
//

void CBUS::CANenumeration(void) {

  // initiate CAN bus enumeration cycle, either due to ENUM opcode or user button press

  if (config.FLiM) {

    // Serial << F("> beginning self-enumeration cycle") << endl;

    // set global variables
    bCANenum = true;                  // we are enumerating
    CANenumTime = millis();           // the cycle start time
    bCANenumComplete = false;         // the 100ms cycle has not completed
    selected_id = 1;
    enums = 0;                        // number of zero-length messages received

    // clear the results array (16 bytes * 8 bits = 128 bits)
    for (byte i = 0; i < 16; i++) {
      enum_responses[i] = 0;
    }

    // send zero-length RTR frame
    memset(&_msg, 0, sizeof(_msg));
    _msg.rtr = true;
    _msg.ext = false;
    _msg.len = 0;
    sendMessage(&_msg);

    // Serial << F("> enumeration cycle initiated") << endl;
  }

  return;
}



//
/// initiate the transition from SLiM to FLiM mode
/// due to long button press in SLiM mode
//

void CBUS::initFLiM(void) {

  // Serial << F("> initiating FLiM negotation") << endl;

  indicateMode(MODE_CHANGING);
  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/SLiM node
  memset(&_msg, 0, sizeof(_msg));
  _msg.rtr = false;
  _msg.ext = false;
  _msg.len = 3;
  _msg.data[0] = OPC_RQNN;
  _msg.data[1] = highByte(config.nodeNum);
  _msg.data[2] = lowByte(config.nodeNum);
  sendMessage(&_msg);

  // Serial << F("> requesting NN with RQNN message for NN = ") << config.nodeNum << endl;
  return;
}

//
/// revert from FLiM to SLiM mode
//

void CBUS::revertSLiM(void) {

  // Serial << F("> reverting to SLiM mode") << endl;

  // send NNREL message
  memset(&_msg, 0, sizeof(_msg));
  _msg.len = 3;
  _msg.rtr = false;
  _msg.ext = false;
  _msg.data[0] = OPC_NNREL;
  _msg.data[1] = highByte(config.nodeNum);
  _msg.data[2] = lowByte(config.nodeNum);

  sendMessage(&_msg);

  // set CANID and NN to zero
  config.setFLiM(0);
  config.setCANID(0);
  config.setNodeNum(0);

  // set indicator LEDs for SLiM mode
  indicateMode(config.FLiM);
  return;
}

//
/// change or re-confirm node number
//

void CBUS::renegotiate(void) {

  initFLiM();
}

//
/// assign the two CBUS LED objects
// 

void CBUS::setLEDs(CBUSLED green, CBUSLED yellow) {

  _ledGrn = green;
  _ledYlw = yellow;

  return;
}

//
/// assign the CBUS pushbutton switch object
//

void CBUS::setSwitch(CBUSSwitch sw) {

  _sw = sw;
}

//
/// set the CBUS LEDs to indicate the current mode
//

void CBUS::indicateMode(byte mode) {

  // Serial << F("> indicating mode = ") << mode << endl;

  switch (mode) {

    case MODE_FLIM:
      _ledYlw.on();
      _ledGrn.off();
      break;

    case MODE_SLIM:
      _ledYlw.off();
      _ledGrn.on();
      break;

    case MODE_CHANGING:
      _ledYlw.blink();
      _ledGrn.off();
      break;

    default:
      break;

  }

  _ledYlw.run();
  _ledGrn.run();

}

/// main CBUS message processing procedure

void CBUS::process(void) {

  static byte remoteCANID = 0, nvindex = 0, nvval = 0, evnum = 0, evindex = 0, evval = 0;
  unsigned int nn = 0, en = 0, j = 0, opc;

  // allow LEDs to update
  _ledGrn.run();
  _ledYlw.run();

  // allow the CBUS some processing time
  _sw.run();

  //
  /// use LEDs to indicate that the user can release the switch
  //

  if (_sw.isPressed() && _sw.getCurrentStateDuration() > SW_TR_HOLD) {
    indicateMode(MODE_CHANGING);
  }

  //
  /// handle switch state changes
  //

  if (_sw.stateChanged()) {

    // has switch been released ?
    if (!_sw.isPressed()) {

      // how long was it pressed for ?
      unsigned long press_time = _sw.getLastStateDuration();

      // long hold > 6 secs
      if (press_time > SW_TR_HOLD) {
        // initiate mode change
        if (!config.FLiM) {
          initFLiM();
        } else {
          revertSLiM();
        }
      }

      // short 1-2 secs
      if (press_time >= 1000 && press_time < 2000) {
        renegotiate();
      }

      // very short < 0.5 sec
      if (press_time < 500 && config.FLiM) {
        CANenumeration();
      }

    } else {
      // do any switch release processing here
    }
  }

  // get received CAN frames from buffer
  while (available()) {

    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    memset(&_msg, 0, sizeof(CANFrame));
    _msg = getNextMessage();

    // extract OPC, NN, EN, remote CANID
    opc = _msg.data[0];
    nn = (_msg.data[1] << 8) + _msg.data[2];
    en = (_msg.data[3] << 8) + _msg.data[4];
    remoteCANID = getCANID(_msg.id);

    //
    /// format and display the frame data payload
    //

    dstr[0] = 0;
    msgstr[0] = 0;

    for (byte b = 0; b < 8; b++) {
      if (b < _msg.len) {
        sprintf(msgstr, "0x%02hx ", _msg.data[b]);
        strcat(dstr, msgstr);
      } else {
        strcat(dstr, "     ");
      }
    }

    sprintf(msgstr, "> CAN frame : CAN ID = [%3hd] len = [%1hd] opc = [0x%02hx] data = [%s] : (%8lu) ", remoteCANID, _msg.len, opc, dstr, millis());
    Serial << msgstr << endl;

    // is this a CANID enumeration request from another node (RTR set) ?
    if (_msg.rtr) {
      
      // Serial << F("> CANID enumeration RTR from CANID = ") << remoteCANID << endl;

      // send an empty message to show our CANID
      memset(&_msg, 0, sizeof(_msg));
      _msg.rtr = false;
      _msg.ext = false;
      _msg.len = 0;
      sendMessage(&_msg);
      continue;
    }

    // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
    if (_msg.ext) {
      // Serial << F("> extended frame ignored, from CANID = ") << remoteCANID << endl;
      continue;
    }

    // are we enumerating CANIDs ?
    if (bCANenum && !bCANenumComplete) {

      //  a frame with zero-length message is an ENUM response
      if (_msg.len == 0) {

        // enumeratiom timer is still running -- process the CANID of this frame
        // Serial << F("> zero - length frame from CANID = ") << remoteCANID << endl;
        ++enums;

        // is there a clash with my current CANID ?
        if (remoteCANID == config.CANID) {
          // Serial << F("> !!! there was a clash with my current CANID !!!") << endl;
        }

        // store this response in the results array
        if (remoteCANID > 0) {
          bitWrite(enum_responses[(remoteCANID / 8)], remoteCANID % 8, 1);
          // Serial << F("> stored CANID ") << remoteCANID << F(" at index = ") << (remoteCANID / 8) << F(", bit = ") << (remoteCANID % 8) << endl;
        }

        continue;
      }
    }

    //
    /// process the message opcode
    /// if we got this far, it's a standard CAN frame (not extended, not RTR) with a data payload length > 0
    //

    if (_msg.len > 0) {

      byte index;

      switch (opc) {

        case OPC_ACON:
        case OPC_ACON1:
        case OPC_ACON2:
        case OPC_ACON3:

        case OPC_ACOF:
        case OPC_ACOF1:
        case OPC_ACOF2:
        case OPC_ACOF3:

        case OPC_ASON:
        case OPC_ASON1:
        case OPC_ASON2:
        case OPC_ASON3:

        case OPC_ASOF:
        case OPC_ASOF1:
        case OPC_ASOF2:
        case OPC_ASOF3:

          // accessory on or off
          // try to find a matching stored event -- match on: nn, en

          index = config.findExistingEvent(nn, en);

          if (index < config.EE_MAX_EVENTS) {
            // do the module-specific action for a CBUS accessory on/off message
            
            if (eventhandler != NULL)
              (void)(*eventhandler)(index, &_msg);

          } else {
            // no matching event for this nn and en
          }

          break;

        case OPC_RQNP:
          // RQNP message - request for node paramters -- does not contain a NN or EN
          // Serial << F("> RQNP -- request for node params during FLiM transition for NN = ") << nn << endl;

          // only respond if we are in transition to FLiM mode
          if (bModeChanging == true) {

            // Serial << F("> responding to RQNP with PARAMS") << endl;

            // respond with PARAMS message
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 8;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_PARAMS;    // opcode
            _msg.data[1] = _mparams[1];     // manf code -- MERG
            _msg.data[2] = _mparams[2];     // minor code ver
            _msg.data[3] = _mparams[3];     // module ident
            _msg.data[4] = _mparams[4];     // number of events
            _msg.data[5] = _mparams[5];     // events vars per event
            _msg.data[6] = _mparams[6];     // number of NVs
            _msg.data[7] = _mparams[7];     // major code ver
            // final param[8] = node flags is not sent here as the max message payload is 8 bytes (0-7)
            sendMessage(&_msg);

          } else {
            // Serial << F("> ignored, not in mode changing mode") << endl;
          }

          break;

        case OPC_PARAMS:
          // another node sending a PARAMS response
          break;

        case OPC_RQNPN:
          // RQNPN message -- request parameter by index number
          // index 0 = number of params available;
          // respond with PARAN

          if (nn == config.nodeNum) {

            byte paran = _msg.data[3];

            // Serial << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

            if (paran <= _mparams[0]) {

              paran = _msg.data[3];

              memset(&_msg, 0, sizeof(_msg));
              _msg.len = 5;
              _msg.rtr = false;
              _msg.ext = false;
              _msg.data[0] = OPC_PARAN;
              _msg.data[1] = highByte(config.nodeNum);
              _msg.data[2] = lowByte(config.nodeNum);
              _msg.data[3] = paran;
              _msg.data[4] = _mparams[paran];
              sendMessage(&_msg);

            } else {
              // Serial << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
              sendCMDERR(9);
            }
          }

          break;

        case OPC_PARAN:
          // another node responding to FCU with a parameter
          break;

        case OPC_SNN:
          // received SNN - set node number
          // Serial << F("> received SNN with NN = ") << nn << endl;

          if (bModeChanging) {
            // Serial << F("> buf[1] = ") << _msg.data[1] << ", buf[2] = " << _msg.data[2] << endl;

            // save the NN
            config.setNodeNum((_msg.data[1] << 8) + _msg.data[2]);

            // respond with NNACK
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 3;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_NNACK;
            _msg.data[1] = highByte(config.nodeNum);
            _msg.data[2] = lowByte(config.nodeNum);

            sendMessage(&_msg);

            // Serial << F("> sent NNACK for NN = ") << config.nodeNum << endl;

            // we are now in FLiM mode - update the configuration
            bModeChanging = false;
            config.setFLiM(true);
            indicateMode(config.FLiM);

            // enumerate the CAN bus to allocate a free CAN ID
            CANenumeration();

            // Serial << F("> FLiM mode = ") << config.FLiM << F(", node number = ") << config.nodeNum << F(", CANID = ") << config.CANID << endl;

          } else {
            Serial << F("> received SNN but not in transition") << endl;
          }

          break;

        case OPC_NNACK:
          // NNACK -- this is another node responding to SNN
          break;

        case OPC_CANID:
          // CAN -- set CANID
          // Serial << F("> CANID for nn = ") << nn << F(" with new CANID = ") << msg.data[3] << endl;

          if (nn == config.nodeNum) {
            // Serial << F("> setting my CANID to ") << CANID << endl;
            config.setCANID(_msg.data[3]);
          }

          break;

        case OPC_ENUM:
          // received ENUM -- start CAN bus self-enumeration
          // Serial << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;

          if (nn == config.nodeNum && remoteCANID != config.CANID && !bCANenum) {
            CANenumeration();
          }

          break;

        case OPC_NVRD:
          // received NVRD -- read NV by index
          if (nn == config.nodeNum) {

            nvindex = _msg.data[3];
            // Serial << F("> NVRD for nn = ") << nn << F(", nv index = ") << nvindex << endl;

            // respond with NVANS
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 5;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_NVANS;
            _msg.data[1] = highByte(config.nodeNum);
            _msg.data[2] = lowByte(config.nodeNum);
            _msg.data[3] = nvindex;
            _msg.data[4] = config.readNV(nvindex);

            sendMessage(&_msg);
          }

          break;

        case OPC_NVSET:
          // received NVSET -- set NV by index
          if (nn == config.nodeNum) {

            nvindex = _msg.data[3];
            nvval = _msg.data[4];
            // Serial << F("> NVSET for index = ") << nvindex << F(", val = ") << nvval << endl;

            // update EEPROM for this NV -- NVs are indexed from 1, not zero
            config.writeNV(nvindex, nvval);

            // respond with WRACK
            sendWRACK();
          }

          break;

        case OPC_NNLRN:
          // received NNLRN -- place into learn mode
          if (nn == config.nodeNum) {
            // Serial << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;
            bLearn = true;
          }

          break;

        case OPC_EVULN:
          // received EVULN -- unlearn an event, by event number
          en = (_msg.data[3] << 8) + _msg.data[4];
          // Serial << F("> EVULN for nn = ") << nn << F(", en = ") << en << endl;

          // we must be in learn mode
          if (bLearn == true) {

            // Serial << F("> searching for existing event to unlearn") << endl;

            // search for this NN and EN pair
            index = config.findExistingEvent(nn, en);

            if (index < config.EE_MAX_EVENTS) {

              // Serial << F("> deleting event at index = ") << index << F(", evs ") << endl;
              config.cleareventEEPROM(j);

              // update hash table
              config.updateEvHashEntry(j);

              // respond with WRACK
              sendWRACK();

            } else {
              // Serial << F("> did not find event to delete") << endl;
              // respond with CMDERR
              sendCMDERR(10);
            }

          } // if in learn mode

          break;

        case OPC_NNULN:
          // received NNULN -- exit from learn mode

          if (nn == config.nodeNum && bLearn == true) {
            bLearn = false;
            // Serial << F("> NNULN for node = ") << nn << F(", learn mode off") << endl;
          }

          break;

        case OPC_RQEVN:
          // received RQEVN -- request for number of stored events
          // Serial << F("> RQEVN -- number of stored events for nn = ") << nn << endl;

          if (nn == config.nodeNum) {

            evnum = config.numEvents();
            // Serial << F("> replying to RQEVN with stored events = ") << evnum << endl;

            // respond with 0x74 NUMEV
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 4;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_NUMEV;
            _msg.data[1] = highByte(config.nodeNum);
            _msg.data[2] = lowByte(config.nodeNum);
            _msg.data[3] = evnum;

            sendMessage(&_msg);
          }

          break;

        case OPC_NERD:
          // request for all stored events
          // Serial << F("> NERD : request all stored events for nn = ") << nn << endl;

          if (nn == config.nodeNum) {

            evnum = 0;

            for (byte i = 0; i < config.EE_MAX_EVENTS; i++) {

              if (config.getEvTableEntry(i) != 0) {

                // it's a valid stored event
                // construct and send a ENRSP message

                // Serial << F("> sending ENRSP reply for event index = ") << i << endl;
                memset(&_msg, 0, sizeof(_msg));
                _msg.len = 8;
                _msg.rtr = false;
                _msg.ext = false;
                _msg.data[0] = OPC_ENRSP;                                                      // response opcode
                _msg.data[1] = highByte(config.nodeNum);                                              // my NN hi
                _msg.data[2] = lowByte(config.nodeNum);                                               // my NN lo
                _msg.data[3] = config.readEEPROM(config.EE_EVENTS_START + (i * config.EE_BYTES_PER_EVENT) + 0);     // event NNhi
                _msg.data[4] = config.readEEPROM(config.EE_EVENTS_START + (i * config.EE_BYTES_PER_EVENT) + 1);     // event NNlo
                _msg.data[5] = config.readEEPROM(config.EE_EVENTS_START + (i * config.EE_BYTES_PER_EVENT) + 2);     // event ENhi
                _msg.data[6] = config.readEEPROM(config.EE_EVENTS_START + (i * config.EE_BYTES_PER_EVENT) + 3);     // event ENlo
                _msg.data[7] = i;                                                              // event table index

                sendMessage(&_msg);
                delay(10);

              } // valid stored ev
            } // loop each ev
          } // for me

          break;

        case OPC_REVAL:
          // received REVAL -- request read of an event variable by event index and ev num
          // respond with NEVAL

          if (nn == config.nodeNum) {

            byte eventidx = _msg.data[3];      // stored event index, from 0
            byte evvaridx = _msg.data[4];      // event var index, from 1

            // Serial << F("> REVAL -- request event variable for nn = ") << nn << F(", eventidx = ") << eventidx << F(", evvaridx = ") << evvaridx << endl;

            if (config.getEvTableEntry(eventidx) != 0) {

              byte evval = ((byte)config.readEEPROM(config.EE_EVENTS_START + (eventidx * config.EE_BYTES_PER_EVENT) + 4 + (evvaridx - 1)));
              // Serial << F("> evval = ") << evval << endl;
              memset(&_msg, 0, sizeof(_msg));
              _msg.len = 6;
              _msg.rtr = false;
              _msg.ext = false;
              _msg.data[0] = OPC_NEVAL;
              _msg.data[1] = highByte(config.nodeNum);
              _msg.data[2] = lowByte(config.nodeNum);
              _msg.data[3] = eventidx;
              _msg.data[4] = evvaridx;
              _msg.data[5] = evval;

              sendMessage(&_msg);
            } else {

              // Serial << F("> request for invalid event index") << endl;
              sendCMDERR(6);
            }

          }

          break;

        case OPC_NNCLR:
          // NNCLR -- clear all stored events

          if (bLearn == true && nn == config.nodeNum) {

            // Serial << F("> NNCLR -- clear all events") << endl;

            for (byte e = 0; e < config.EE_MAX_EVENTS; e++) {
              config.cleareventEEPROM(e);
            }

            // recreate the hash table
            config.clearEvHashTable();
            // Serial << F("> cleared all events") << endl;

            sendWRACK();
          }

          break;

        case OPC_NEVAL:
          // this is another node responding to REVAL
          break;

        case OPC_NNEVN:
          // request for number of free event slots

          if (config.nodeNum == nn) {

            byte free_slots = 0;

            for (byte i = 0; i < config.EE_MAX_EVENTS; i++) {
              if (config.getEvTableEntry(i) == 0) {
                ++free_slots;
              }
            }

            // Serial << F("> responding to to NNEVN with EVNLF, free event table slots = ") << free_slots << endl;
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 4;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_EVNLF;
            _msg.data[1] = highByte(config.nodeNum);
            _msg.data[2] = lowByte(config.nodeNum);
            _msg.data[3] = free_slots;
            sendMessage(&_msg);
          }

          break;

        case OPC_EVNLF:
          // another node responding to NNEVN
          break;

        case OPC_QNN:
          // this is probably a config recreate -- respond with PNN if we have a node number
          // Serial << F("> QNN received") << endl;

          if (config.nodeNum > 0) {
            // Serial << ("> responding with PNN message") << endl;
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 6;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_PNN;
            _msg.data[1] = highByte(config.nodeNum);
            _msg.data[2] = lowByte(config.nodeNum);
            _msg.data[3] = _mparams[1];
            _msg.data[4] = _mparams[3];
            _msg.data[5] = _mparams[8];
            sendMessage(&_msg);
          }

          break;

        case OPC_NVANS:
          // another node responding to NVRD
          break;

        case OPC_WRACK:
          // another node sending a write ack
          break;

        case OPC_RQMN:
          // request for node module name, excluding "CAN" prefix
          // Serial << F("> RQMN for nn = ") << nn << endl;

          if (nn == config.nodeNum) {
            // respond with NAME
            memset(&_msg, 0, sizeof(_msg));
            _msg.len = 8;
            _msg.rtr = false;
            _msg.ext = false;
            _msg.data[0] = OPC_NAME;
            memcpy(_msg.data + 1, _mname, 7);
            sendMessage(&_msg);
          }

          break;

        case OPC_RQNN:
          // request for NN from another node
          break;

        case OPC_NUMEV:
          // a node is responding to a RQEVN request with a NUMEV reply
          break;

        case OPC_ENRSP:
          //  a node is responding to a NERD request with a ENRSP reply
          break;

        case OPC_EVLRN:
          // received EVLRN -- learn an event
          evindex = _msg.data[5];
          evval = _msg.data[6];

          // Serial << endl << F("> EVLRN for source nn = ") << nn << F(", en = ") << en << F(", evindex = ") << evindex << F(", evval = ") << evval << endl;

          // we must be in learn mode
          if (bLearn == true) {

            // search for this NN, EN as we may just be adding an EV to an existing learned event
            // Serial << F("> searching for existing event to update") << endl;
            index = config.findExistingEvent(nn, en);

            // not found - it's a new event
            if (index >= config.EE_MAX_EVENTS) {
              // Serial << F("> existing event not found - creating a new one if space available") << endl;
              index = config.findEventSpace();
            }

            // if existing or new event found, write the event data

            // TODO: can this be optimised to reduce EEPROM access ?? e.g. write NN/EN once on first access ??

            if (index < config.EE_MAX_EVENTS) {

              // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
              // Serial << F("> writing EV = ") << evindex << F(", at index = ") << index << F(", offset = ") << (EE_EVENTS_START + (index * EE_BYTES_PER_EVENT)) << endl;

              config.writeEEPROM(config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 0, highByte(nn));
              config.writeEEPROM(config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 1, lowByte(nn));
              config.writeEEPROM(config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 2, highByte(en));
              config.writeEEPROM(config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 3, lowByte(en));
              config.writeEEPROM(config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 4 + (evindex - 1), evval);

              // recreate event hash table entry
              // Serial << F("> updating hash table entry for idx = ") << index << endl;
              config.updateEvHashEntry(index);

              // respond with WRACK
              sendWRACK();

            } else {
              Serial << F("> no free event storage, index = ") << index << endl;
              // // TODO  should we flash the yellow LED ??
              // respond with CMDERR
              sendCMDERR(10);
            }

          } else { // bLearn == true
            // Serial << F("> error -- not in learn mode") << endl;
          }

          break;

        case OPC_AREQ:
          // AREQ message - request for node state, only producer nodes
          break;

        case OPC_BOOT:
          // boot mode -- receive a file and store in eeprom for the bootloader to find on reset
          if (nn == config.nodeNum) {
            // Serial << F("> received BOOT (0x5c) opcode") << endl;
          }

          break;

        case OPC_RSTAT:
          // command station status -- not applicable to modules
          // Serial << F("> RSTAT command station status query") << endl;
          break;

        case OPC_PNN:
          // a node responding to QNN
          break;

        case OPC_ARST:
          // system reset
          config.reboot();
          break;

        default:
          // unknown or unhandled OPC
          // Serial << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
          break;
      }
    } else {
      // Serial << F("> oops ... zero - length frame ?? ") << endl;
    }
  }

  // check CAN bus enumeration timer
  checkCANenum();

  //
  /// check 30 sec timeout for SLiM/FLiM negotiation with FCU
  //

  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // Serial << F("> timeout expired, FLiM = ") << FLiM << F(", mode change = ") << bModeChanging << endl;
    indicateMode(config.FLiM);
    bModeChanging = false;
  }

  // Serial << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of CBUS message processing
  //

  return;
}

void CBUS::checkCANenum(void) {

  //
  /// check the 100ms CAN enumeration cycle timer
  //

  if (bCANenum && !bCANenumComplete && (millis() - CANenumTime) >= 100) {

    // enumeration timer has expired -- stop enumeration and process the responses

    // Serial << F("> enum cycle complete at ") << millis() << F(", start = ") << CANenumTime << F(", duration = ") << (millis() - CANenumTime) << endl;
    // Serial << F("> processing received responses") << endl;

    // iterate through the 128 bit field
    for (byte i = 0; i < 16; i++) {

      // for each bit in the byte
      for (byte b = 0; b < 8; b++) {

        // ignore first bit of first byte -- CAN ID zero is not used for nodes
        if (i == 0 && b == 0) {
          continue;
        }

        // ignore if this byte is all 1's -> there are no unused IDs in this group of numbers
        if (enum_responses[i] == 0xff) {
          continue;
        }

        // if the bit is not set
        if (bitRead(enum_responses[i], b) == 0) {
          selected_id = ((i * 16) + b);
          // Serial << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
          i = 16; // ugh ... but probably better than a goto :)
          break;
        }
      }
    }

    // Serial << F("> enumeration responses = ") << enums << F(", lowest available CAN id = ") << selected_id << endl;

    bCANenumComplete = true;
    bCANenum = false;
    CANenumTime = 0L;

    // store the new CAN ID
    config.setCANID(selected_id);

    // send NNACK to FCU
    memset(&_msg, 0, sizeof(_msg));
    _msg.rtr = false;
    _msg.ext = false;
    _msg.len = 3;
    _msg.data[0] = OPC_NNACK;
    _msg.data[1] = highByte(config.nodeNum);
    _msg.data[2] = lowByte(config.nodeNum);
    sendMessage(&_msg);

  }
}

//
/// GridConnect utilities
/// work in progress, not currently used
//

//
/// convert to and from GridConnect format
//

char *FrameToGridConnect(const CANFrame *frame, char dest[]) {

  // example :SB020N520104;

  byte index = 0;
  char tbuff[4] = {};

  dest[0] = ':';

  // standard or extended frame
  if ((frame->id & 0x80000000) == 0x80000000) {
    dest[1] = 'X';
    index = 8;
  } else {
    dest[1] = 'S';
    index = 6;
  }

  // FIXME: TODO header
  dest[2] = '-';
  dest[3] = '-';
  dest[4] = '-';
  dest[5] = '-';

  // normal or RTR frame
  if ((frame->id & 0x40000000) == 0x40000000) {
    dest[index] = 'R';
  } else {
    dest[index] = 'N';
  }

  dest[index + 1] = '\0';

  // data bytes
  for (byte i = 0; i < frame->len; i++) {
    sprintf(tbuff, "%02X", frame->data[i]);
    strcat(dest, tbuff);
  }

  strcat(dest, ";");
  return dest;
}

CANFrame *GridConnectToFrame(const char string[], CANFrame *msg) {

  byte index = 0;

  memset(msg, 0, sizeof(CANFrame));

  // extended or standard frame
  if (string[1] == 'X') {

    // parse four two-digit hex-encoded blocks into bytes
    msg->id  = ascii_pair_to_byte(string + 2) << 24;
    msg->id += ascii_pair_to_byte(string + 4) << 16;
    msg->id += ascii_pair_to_byte(string + 6) << 8;
    msg->id += ascii_pair_to_byte(string + 8) << 0;

    // set EXT header bit
    msg->id |= 0x80000000;
    index = 10;

  } else if (string[1] == 'S') {

    // parse two two-digit hex-encoded blocks into bytes
    msg->id  = ascii_pair_to_byte(string + 2) << 4;
    msg->id |= ascii_pair_to_byte(string + 3) << 0;

    index = 5;

  } else {
    Serial << F("> GridConnectToFrame: unexpected char, not X or S") << endl;
  }

  if (string[index] == 'N') {
    ++index;
  } else if (string[index] == 'R') {

    // set the RTR header bit
    msg->id |= 0x40000000;
    ++index;
  } else {
    Serial << F("> GridConnectToFrame: unexpected char, not N or R") << endl;
  }

  // get the data
  byte i;

  for (i = 0; string[index] != ';'; index += 2, i++) {
    msg->data[i] = ascii_pair_to_byte(string + index);
  }

  msg->len = i;

  return msg;
}


unsigned long ascii_pair_to_byte(const char *pair) {

  unsigned char* data = (unsigned char*)pair;
  unsigned long result;

  if (data[1] < 'A') {
    result = data[1] - '0';
  } else {
    result = data[1] - 'A' + 10;
  }

  if (data[0] < 'A') {
    result += (data[0] - '0') << 4;
  } else {
    result += (data[0] - 'A' + 10) << 4;
  }

  return result;
}

