
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

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>
#include "CbusService.h"

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

Controller::Controller(Service * service)
  : service(service)
{
  extern Configuration config;
  module_config = &config;

  service->setController(this);
}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//

Controller::Controller(Configuration *the_config, Service * service)
  : module_config(the_config), service(service)
{
  service->setController(this);
}

//
/// register the user handler for learned events
//

// overloaded form which receives the opcode on/off state and the first event variable

//
/// register the user handler for CAN frames
/// default args in .h declaration for opcodes array (NULL) and size (0)
//

void Controller::setFrameHandler(void (*fptr)(CANFrame *msg), byte opcodes[], byte num_opcodes) {
  framehandler = fptr;
  _opcodes = opcodes;
  _num_opcodes = num_opcodes;
}

//
/// assign the module parameter set
//

void Controller::setParams(unsigned char *mparams) {
  _mparams = mparams;
}

//
/// assign the module name
//

void Controller::setName(unsigned char *mname) {
  _mname = mname;
}

//
/// set module to SLiM mode
//

void Controller::setSLiM(void) {

  bModeChanging = false;
  module_config->setNodeNum(0);
  module_config->setModuleMode(MODE_SLIM);
  module_config->setCANID(0);

  indicateMode(MODE_SLIM);
}

//
/// extract CANID from CAN frame header
//

inline byte Controller::getCANID(unsigned long header) {

  return header & 0x7f;
}

//
/// send a WRACK (write acknowledge) message
//

bool Controller::sendWRACK(void) {

  // send a write acknowledgement response

  _msg.len = 3;
  _msg.data[0] = OPC_WRACK;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);

  return sendMessage(&_msg);
}

//
/// send a CMDERR (command error) message
//

bool Controller::sendCMDERR(byte cerrno) {

  // send a command error response

  _msg.len = 4;
  _msg.data[0] = OPC_CMDERR;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);
  _msg.data[3] = cerrno;

  return sendMessage(&_msg);
}

//
/// is this an Extended CAN frame ?
//

bool Controller::isExt(CANFrame *amsg) {

  return (amsg->ext);
}

//
/// is this a Remote frame ?
//

bool Controller::isRTR(CANFrame *amsg) {

  return (amsg->rtr);
}

//
/// if in FLiM mode, initiate a CAN ID enumeration cycle
//

void Controller::CANenumeration(void) {

  // initiate CAN bus enumeration cycle, either due to ENUM opcode, ID clash, or user button press

  // DEBUG_SERIAL << F("> beginning self-enumeration cycle") << endl;

  // set global variables
  bCANenum = true;                  // we are enumerating
  CANenumTime = millis();           // the cycle start time
  memset(enum_responses, 0, sizeof(enum_responses));

  // send zero-length RTR frame
  _msg.len = 0;
  sendMessage(&_msg, true, false);          // fixed arg order in v 1.1.4, RTR - true, ext = false

  // DEBUG_SERIAL << F("> enumeration cycle initiated") << endl;
}

//
/// initiate the transition from SLiM to FLiM mode
//

void Controller::initFLiM(void) {

  // DEBUG_SERIAL << F("> initiating FLiM negotation") << endl;

  indicateMode(MODE_CHANGING);

  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/SLiM node
  _msg.len = 3;
  _msg.data[0] = OPC_RQNN;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);
  sendMessage(&_msg);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

//
/// revert from FLiM to SLiM mode
//

void Controller::revertSLiM(void) {

  // DEBUG_SERIAL << F("> reverting to SLiM mode") << endl;

  // send NNREL message
  _msg.len = 3;
  _msg.data[0] = OPC_NNREL;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);

  sendMessage(&_msg);
  setSLiM();
}

//
/// change or re-confirm node number
//

void Controller::renegotiate(void) {

  initFLiM();
}

void Controller::setUI(UserInterface *ui)
{
  _ui = ui;
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(byte mode) {

  // DEBUG_SERIAL << F("> indicating mode = ") << mode << endl;
  if (_ui) {
    _ui->indicateMode(mode);
  }
}

void Controller::indicateActivity()
{
  if (_ui) {
    _ui->indicateActivity();
  }
}

/// main Controller message processing procedure

void Controller::process(byte num_messages)
{
  // start bus enumeration if required
  if (enumeration_required) {
    enumeration_required = false;
    CANenumeration();
  }

  // process switch operations if the module is configured with one

  if (_ui)
  {
    // allow LEDs and switch to update
    _ui->run();

    //
    /// use LEDs to indicate that the user can release the switch
    //

    if (_ui->resetRequested()) {
      //DEBUG_SERIAL << "> Button is pressed for mode change" << endl;
      indicateMode(MODE_CHANGING);
    }

    performRequestedUserAction();
  }

  // get received CAN frames from buffer
  // process by default 3 messages per run so the user's application code doesn't appear unresponsive under load

  for (byte mcount = 0 ; transport->available() && mcount < num_messages ; ++mcount)
  {
    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    // memset(&_msg, 0, sizeof(CANFrame));
    _msg = transport->getNextMessage();

    unsigned int opc = _msg.data[0];
    byte remoteCANID = getCANID(_msg.id);

    callFrameHandler(opc, &_msg);

    indicateActivity();

    // is this a CANID enumeration request from another node (RTR set) ?
    if (_msg.rtr) {
      // DEBUG_SERIAL << F("> CANID enumeration RTR from CANID = ") << remoteCANID << endl;
      // send an empty message to show our CANID
      _msg.len = 0;
      sendMessage(&_msg);
      continue;
    }

    //
    /// set flag if we find a CANID conflict with the frame's producer
    /// doesn't apply to RTR or zero-length frames, so as not to trigger an enumeration loop
    //

    if (remoteCANID == module_config->CANID && _msg.len > 0) {
      // DEBUG_SERIAL << F("> CAN id clash, enumeration required") << endl;
      enumeration_required = true;
    }

    // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
    if (_msg.ext) {
      // DEBUG_SERIAL << F("> extended frame ignored, from CANID = ") << remoteCANID << endl;
      continue;
    }

    // are we enumerating CANIDs ?
    if (bCANenum && _msg.len == 0) {

      // store this response in the responses array
      if (remoteCANID > 0) {
        // fix to correctly record the received CANID
        bitWrite(enum_responses[(remoteCANID / 16)], remoteCANID % 8, 1);
        // DEBUG_SERIAL << F("> stored CANID ") << remoteCANID << F(" at index = ") << (remoteCANID / 8) << F(", bit = ") << (remoteCANID % 8) << endl;
      }

      continue;
    }

    //
    /// process the message opcode
    /// if we got this far, it's a standard CAN frame (not extended, not RTR) with a data payload length > 0
    //

    if (_msg.len > 0) {
      service->handleMessage(opc, &_msg, remoteCANID);
    } else {
      // DEBUG_SERIAL << F("> oops ... zero - length frame ?? ") << endl;
    }
  }  // while messages available

  // check CAN bus enumeration timer
  checkCANenum();

  //
  /// check 30 sec timeout for SLiM/FLiM negotiation with FCU
  //

  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // Revert to previous mode.
    // DEBUG_SERIAL << F("> timeout expired, currentMode = ") << currentMode << F(", mode change = ") << bModeChanging << endl;
    indicateMode(module_config->currentMode);
    bModeChanging = false;
  }

  // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of Controller message processing
  //
}

void Controller::performRequestedUserAction()
{
  //
  /// handle switch state changes
  //

  UserInterface::RequestedAction requestedAction = _ui->checkRequestedAction();
  switch (requestedAction)
  {
    case UserInterface::CHANGE_MODE:
      // initiate mode change
      //Serial << "Controller::process() - changing mode, current mode=" << module_config->currentMode << endl;
      if (!module_config->currentMode) {
        initFLiM();
      } else {
        revertSLiM();
      }
      break;

    case UserInterface::RENEGOTIATE:
      //Serial << "Controller::process() - renegotiate" << endl;
      renegotiate();
      break;

    case UserInterface::ENUMERATION:
      //Serial << "Controller::process() - enumerate" << endl;
      if (module_config->currentMode)
      {
        CANenumeration();
      }
      break;

    case UserInterface::NONE:
      //Serial << "Controller::process() - no action" << endl;
      break;
  }
}

void Controller::callFrameHandler(unsigned int opc, CANFrame *msg)
{
  if (framehandler != NULL) {

    // check if incoming opcode is in the user list, if list length > 0
    if (_num_opcodes > 0) {
      for (byte i = 0; i < _num_opcodes; i++) {
        if (opc == _opcodes[i]) {
          (void)(*framehandler)(msg);
          break;
        }
      }
    } else {
      (void)(*framehandler)(msg);
    }
  }
}

void Controller::checkCANenum(void)
{
  //
  /// check the 100ms CAN enumeration cycle timer
  //
  if (bCANenum && (millis() - CANenumTime) >= 100)
  {
    // enumeration timer has expired -- stop enumeration and process the responses

    // DEBUG_SERIAL << F("> enum cycle complete at ") << millis() << F(", start = ") << CANenumTime << F(", duration = ") << (millis() - CANenumTime) << endl;
    // DEBUG_SERIAL << F("> processing received responses") << endl;

    byte selected_id = findFreeCanId();

    // DEBUG_SERIAL << F("> enumeration responses = ") << enums << F(", lowest available CAN id = ") << selected_id << endl;

    bCANenum = false;
    CANenumTime = 0UL;

    // store the new CAN ID
    module_config->setCANID(selected_id);

    // send NNACK
    _msg.len = 3;
    _msg.data[0] = OPC_NNACK;
    _msg.data[1] = highByte(module_config->nodeNum);
    _msg.data[2] = lowByte(module_config->nodeNum);
    sendMessage(&_msg);
  }
}

byte Controller::findFreeCanId()
{
  // iterate through the 128 bit field
  for (byte i = 0; i < 16; i++) {

    // ignore if this byte is all 1's -> there are no unused IDs in this group of numbers
    if (enum_responses[i] == 0xff) {
      continue;
    }

    // for each bit in the byte
    for (byte b = 0; b < 8; b++) {

      // ignore first bit of first byte -- CAN ID zero is not used for nodes
      if (i == 0 && b == 0) {
        continue;
      }

      // if the bit is not set
      if (bitRead(enum_responses[i], b) == 0) {
        byte selected_id = ((i * 16) + b);
        // DEBUG_SERIAL << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
        return selected_id;
      }
    }
  }

  return 1;     // default if no responses from other modules
}

//
/// for accessory event messages, lookup the event in the event table and call the user's registered event handler function
//

//
/// set the long message handler object to receive long message frames
//

void Controller::setLongMessageHandler(LongMessageController *handler) {
  longMessageHandler = handler;
}

}