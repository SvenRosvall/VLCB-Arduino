// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

Controller::Controller(UserInterface * ui, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , transport(trpt)
  , services(services)
{
  extern Configuration config;
  module_config = &config;

  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//
Controller::Controller(UserInterface * ui, Configuration *conf, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , module_config(conf)
  , transport(trpt)
  , services(services)
{
  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

bool Controller::sendMessage(CANFrame *msg, bool rtr, bool ext, byte priority)
{
  msg->id = getModuleCANID();
  return transport->sendMessage(msg, rtr, ext, priority);
}

//
/// register the user handler for learned events
//

// overloaded form which receives the opcode on/off state and the first event variable

//
/// register the user handler for CAN frames
/// default args in .h declaration for opcodes array (NULL) and size (0)
//
void Controller::setFrameHandler(void (*fptr)(CANFrame *msg), byte opcodes[], byte num_opcodes)
{
  framehandler = fptr;
  _opcodes = opcodes;
  _num_opcodes = num_opcodes;
}

//
/// assign the module parameter set
//
void Controller::setParams(unsigned char *mparams)
{
  _mparams = mparams;
}

//
/// assign the module name
//
void Controller::setName(unsigned char *mname)
{
  _mname = mname;
}

//
/// extract CANID from CAN frame header
//
byte Controller::getCANID(unsigned long header)
{
  return header & 0x7f;
}

//
/// send a WRACK (write acknowledge) message
//
bool Controller::sendWRACK()
{
  // send a write acknowledgement response
  return sendMessageWithNN(OPC_WRACK);
}

//
/// send a CMDERR (command error) message
//
bool Controller::sendCMDERR(byte cerrno)
{
  // send a command error response
  return sendMessageWithNN(OPC_CMDERR, cerrno);
}

//
/// is this an Extended CAN frame ?
//
bool Controller::isExt(CANFrame *amsg)
{
  return (amsg->ext);
}

//
/// is this a Remote frame ?
//
bool Controller::isRTR(CANFrame *amsg)
{
  return (amsg->rtr);
}

//
/// if in FLiM mode, initiate a CAN ID enumeration cycle
//
void Controller::startCANenumeration()
{
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
void Controller::initFLiM()
{
  // DEBUG_SERIAL << F("> initiating FLiM negotation") << endl;

  indicateMode(MODE_CHANGING);

  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/SLiM node
  sendMessageWithNN(OPC_RQNN);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

void Controller::setFLiM()
{
  // DEBUG_SERIAL << F("> set FLiM") << endl;
  bModeChanging = false;
  module_config->setModuleMode(MODE_FLIM);
  indicateMode(MODE_FLIM);

  // enumerate the CAN bus to allocate a free CAN ID
  startCANenumeration();
}

//
/// set module to SLiM mode
//
void Controller::setSLiM()
{
  // DEBUG_SERIAL << F("> set SLiM") << endl;
  bModeChanging = false;
  module_config->setNodeNum(0);
  module_config->setModuleMode(MODE_SLIM);
  module_config->setCANID(0);

  indicateMode(MODE_SLIM);
}

//
/// revert from FLiM to SLiM mode
//
void Controller::revertSLiM()
{

  // DEBUG_SERIAL << F("> reverting to SLiM mode") << endl;

  // send NNREL message

  sendMessageWithNN(OPC_NNREL);
  setSLiM();
}

//
/// check 30 sec timeout for SLiM/FLiM negotiation with FCU
//
void Controller::checkModeChangeTimeout()
{
  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // Revert to previous mode.
    // DEBUG_SERIAL << F("> timeout expired, currentMode = ") << currentMode << F(", mode change = ") << bModeChanging << endl;
    indicateMode(module_config->currentMode);
    bModeChanging = false;
  }
}

//
/// change or re-confirm node number
//

void Controller::renegotiate()
{
  initFLiM();
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(byte mode)
{
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

//
/// main Controller message processing procedure
//
void Controller::process(byte num_messages)
{
  // TODO: Move to CanService
  if (enumeration_required)
  {
    enumeration_required = false;
    startCANenumeration();
  }

  // process switch operations if the module is configured with one

  if (_ui)
  {
    _ui->run();

    performRequestedUserAction(_ui->checkRequestedAction());
  }

  // get received CAN frames from buffer
  // process by default 3 messages per run so the user's application code doesn't appear unresponsive under load

  for (byte mcount = 0 ; transport->available() && mcount < num_messages ; ++mcount)
  {
    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    _msg = transport->getNextMessage();

    // TODO: Move to CanService
    byte remoteCANID = getCANID(_msg.id);

    callFrameHandler(&_msg);

    indicateActivity();

    // is this a CANID enumeration request from another node (RTR set) ?
    // TODO: Move to CanService
    if (_msg.rtr)
    {
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
    // TODO: Move to CanService
    if (remoteCANID == module_config->CANID && _msg.len > 0)
    {
      // DEBUG_SERIAL << F("> CAN id clash, enumeration required") << endl;
      enumeration_required = true;
    }

    // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
    if (_msg.ext)
    {
      // DEBUG_SERIAL << F("> extended frame ignored, from CANID = ") << remoteCANID << endl;
      continue;
    }

    // are we enumerating CANIDs ?
    // TODO: Move to CanService
    if (bCANenum && _msg.len == 0)
    {

      // store this response in the responses array
      if (remoteCANID > 0)
      {
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

    if (_msg.len > 0) 
    {
      unsigned int opc = _msg.data[0];
      for (Service * service : services)
      {
        if (service->handleMessage(opc, &_msg) == PROCESSED)
        {
          break;
        }
      }
    } else {
      // DEBUG_SERIAL << F("> oops ... zero - length frame ?? ") << endl;
    }
  }  // while messages available

  // TODO: Move to CanService
  checkCANenumTimout();

  checkModeChangeTimeout();

  // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of Controller message processing
  //
}

void Controller::performRequestedUserAction(UserInterface::RequestedAction requestedAction)
{
  switch (requestedAction)
  {
    case UserInterface::CHANGE_MODE:
      // initiate mode change
      //Serial << "Controller::process() - changing mode, current mode=" << module_config->currentMode << endl;
      if (!module_config->currentMode)
      {
        initFLiM();
      }
      else
      {
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
        startCANenumeration();
      }
      break;

    case UserInterface::NONE:
      //Serial << "Controller::process() - no action" << endl;
      break;
  }
}

// Return true if framehandler shall be called for registered opcodes, if any.
bool Controller::filterByOpcodes(const CANFrame *msg) const
{
  if (_num_opcodes == 0)
  {
    return true;
  }
  if (msg->len > 0)
  {
    unsigned int opc = msg->data[0];
    for (byte i = 0; i < _num_opcodes; i++)
    {
      if (opc == _opcodes[i])
      {
        return true;
      }
    }
  }
  return false;
}

void Controller::callFrameHandler(CANFrame *msg)
{
  if (framehandler != NULL)
  {
    if (filterByOpcodes(msg))
    {
      (void)(*framehandler)(msg);
    }
  }
}

void Controller::checkCANenumTimout()
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
    sendMessageWithNN(OPC_NNACK);
  }
}

byte Controller::findFreeCanId()
{
  // iterate through the 128 bit field
  for (byte i = 0; i < 16; i++)
  {
    // ignore if this byte is all 1's -> there are no unused IDs in this group of numbers
    if (enum_responses[i] == 0xff)
    {
      continue;
    }

    // for each bit in the byte
    for (byte b = 0; b < 8; b++)
    {
      // ignore first bit of first byte -- CAN ID zero is not used for nodes
      if (i == 0 && b == 0) {
        continue;
      }

      // if the bit is not set
      if (bitRead(enum_responses[i], b) == 0)
      {
        byte selected_id = ((i * 16) + b);
        // DEBUG_SERIAL << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
        return selected_id;
      }
    }
  }

  return 1;     // default if no responses from other modules
}

void setNN(CANFrame *msg, unsigned int nn)
{
  msg->data[1] = highByte(nn);
  msg->data[2] = lowByte(nn);
}

bool Controller::sendMessageWithNN(int opc)
{
  CANFrame msg;
  msg.len = 3;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1)
{
  CANFrame msg;
  msg.len = 4;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2)
{
  CANFrame msg;
  msg.len = 5;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  msg.data[4] = b2;
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3)
{
  CANFrame msg;
  msg.len = 6;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  msg.data[4] = b2;
  msg.data[5] = b3;
  return sendMessage(&msg);
}

}