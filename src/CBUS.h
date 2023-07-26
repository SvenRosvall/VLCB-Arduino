
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

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include <SPI.h>

#include <CBUSconfig.h>
#include <cbusdefs.h>
#include <CBUSTransport.h>
#include "UserInterface.h"

#define LONG_MESSAGE_DEFAULT_DELAY 20      // delay in milliseconds between sending successive long message fragments
#define LONG_MESSAGE_RECEIVE_TIMEOUT 5000  // timeout waiting for next long message packet
#define NUM_EX_CONTEXTS 4                  // number of send and receive contexts for extended implementation = number of concurrent messages
#define EX_BUFFER_LEN 64                   // size of extended send and receive buffers
#define OPC_DTXC 0xe9                      // temp opcode for CBUS long message

//
/// CBUS modes
//

enum {
  MODE_SLIM = 0,
  MODE_FLIM = 1,
  MODE_CHANGING = 2
};

//
/// CBUS long message status codes
//

enum {
  CBUS_LONG_MESSAGE_INCOMPLETE = 0,
  CBUS_LONG_MESSAGE_COMPLETE,
  CBUS_LONG_MESSAGE_SEQUENCE_ERROR,
  CBUS_LONG_MESSAGE_TIMEOUT_ERROR,
  CBUS_LONG_MESSAGE_CRC_ERROR,
  CBUS_LONG_MESSAGE_TRUNCATED
};

//
/// CAN/CBUS message type
//

class CANFrame {

public:
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8] = {};
};

//
/// an abstract class to encapsulate CAN bus and CBUS processing
/// it must be implemented by a derived subclass
//

class CBUSLongMessage;      // forward reference

class CBUS {

public:
  CBUS();
  CBUS(CBUSConfig *the_config);
  void setTransport(CBUSTransport * transport) { this->transport = transport; }

  // TODO: These methods deal with transportation. While refactoring they delegate to the transport.

  bool available(void)
  {
    return transport->available();
  }
  bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY)
  {
    return transport->sendMessage(msg, rtr, ext, priority);
  }

  // implementations of these methods are provided in the base class

  bool sendWRACK(void);
  bool sendCMDERR(byte cerrno);
  void CANenumeration(void);
  byte getCANID(unsigned long header);
  byte getModuleCANID() { return module_config->CANID; }
  bool isExt(CANFrame *msg);
  bool isRTR(CANFrame *msg);
  void process(byte num_messages = 3);
  void initFLiM(void);
  void revertSLiM(void);
  void setSLiM(void);
  void renegotiate(void);
  void setUI(UserInterface *ui);
  void setParams(unsigned char *mparams);
  void setName(unsigned char *mname);
  void checkCANenum(void);
  void indicateMode(byte mode);
  void indicateActivity();
  void setEventHandler(void (*fptr)(byte index, CANFrame *msg));
  void setEventHandler(void (*fptr)(byte index, CANFrame *msg, bool ison, byte evval));
  void setFrameHandler(void (*fptr)(CANFrame *msg), byte *opcodes = NULL, byte num_opcodes = 0);
  void processAccessoryEvent(unsigned int nn, unsigned int en, bool is_on_event);

  void setLongMessageHandler(CBUSLongMessage *handler);

protected:                                          // protected members become private in derived classes
  CANFrame _msg;
  UserInterface *_ui;
  CBUSConfig *module_config;
  unsigned char *_mparams;
  unsigned char *_mname;
  void (*eventhandler)(byte index, CANFrame *msg);
  void (*eventhandlerex)(byte index, CANFrame *msg, bool evOn, byte evVal);
  void (*framehandler)(CANFrame *msg);
  byte *_opcodes;
  byte _num_opcodes;
  byte enum_responses[16];                          // 128 bits for storing CAN ID enumeration results
  bool bModeChanging, bCANenum, bLearn;
  unsigned long timeOutTimer, CANenumTime;
  bool enumeration_required;

  CBUSTransport * transport;

  CBUSLongMessage *longMessageHandler = NULL;       // CBUS long message object to receive relevant frames
};

//
/// a basic class to send and receive CBUS long messages per MERG RFC 0005
/// handles a single message, sending and receiving
/// suitable for small microcontrollers with limited memory
//

class CBUSLongMessage {

public:

  CBUSLongMessage(CBUS *cbus_object_ptr);
  bool sendLongMessage(const void *msg, const unsigned int msg_len, const byte stream_id, const byte priority = DEFAULT_PRIORITY);
  void subscribe(byte *stream_ids, const byte num_stream_ids, void *receive_buffer, const unsigned int receive_buffer_len, void (*messagehandler)(void *fragment, const unsigned int fragment_len, const byte stream_id, const byte status));
  bool process(void);
  virtual void processReceivedMessageFragment(const CANFrame *frame);
  bool is_sending(void);
  void setDelay(byte delay_in_millis);
  void setTimeout(unsigned int timeout_in_millis);

protected:

  bool sendMessageFragment(CANFrame *frame, const byte priority);

  bool _is_receiving = false;
  byte *_send_buffer, *_receive_buffer;
  byte _send_stream_id = 0, _receive_stream_id = 0, *_stream_ids = NULL, _num_stream_ids = 0, _send_priority = DEFAULT_PRIORITY, _msg_delay = LONG_MESSAGE_DEFAULT_DELAY, _sender_canid = 0;
  unsigned int _send_buffer_len = 0, _incoming_message_length = 0, _receive_buffer_len = 0, _receive_buffer_index = 0, _send_buffer_index = 0, _incoming_message_crc = 0, \
                                  _incoming_bytes_received = 0, _receive_timeout = LONG_MESSAGE_RECEIVE_TIMEOUT, _send_sequence_num = 0, _expected_next_receive_sequence_num = 0;
  unsigned long _last_fragment_sent = 0UL, _last_fragment_received = 0UL;

  void (*_messagehandler)(void *fragment, const unsigned int fragment_len, const byte stream_id, const byte status);        // user callback function to receive long message fragments
  CBUS *_cbus_object_ptr;
};


//// extended support for multiple concurrent long messages

// send and receive contexts

struct receive_context_t {
  bool in_use;
  byte receive_stream_id, sender_canid;
  byte *buffer;
  unsigned int receive_buffer_index, incoming_bytes_received, incoming_message_length, expected_next_receive_sequence_num, incoming_message_crc;
  unsigned long last_fragment_received;
};

struct send_context_t {
  bool in_use;
  byte send_stream_id, send_priority, msg_delay;
  byte *buffer;
  unsigned int send_buffer_len, send_buffer_index, send_sequence_num;
  unsigned long last_fragment_sent;
};

//
/// a derived class to extend the base long message class to handle multiple concurrent messages, sending and receiving
//

class CBUSLongMessageEx : public CBUSLongMessage {

public:

  CBUSLongMessageEx(CBUS *cbus_object_ptr)
    : CBUSLongMessage(cbus_object_ptr) {}         // derived class constructor calls the base class constructor

  bool allocateContexts(byte num_receive_contexts = NUM_EX_CONTEXTS, unsigned int receive_buffer_len = EX_BUFFER_LEN, byte num_send_contexts = NUM_EX_CONTEXTS);
  bool sendLongMessage(const void *msg, const unsigned int msg_len, const byte stream_id, const byte priority = DEFAULT_PRIORITY);
  bool process(void);
  void subscribe(byte *stream_ids, const byte num_stream_ids, void (*messagehandler)(void *msg, unsigned int msg_len, byte stream_id, byte status));
  virtual void processReceivedMessageFragment(const CANFrame *frame);
  byte is_sending(void);
  void use_crc(bool use_crc);

private:

  bool _use_crc = false;
  byte _num_receive_contexts = NUM_EX_CONTEXTS, _num_send_contexts = NUM_EX_CONTEXTS;
  receive_context_t **_receive_context = NULL;
  send_context_t **_send_context = NULL;
};

