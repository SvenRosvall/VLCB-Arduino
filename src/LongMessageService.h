#pragma once

#include "Service.h"
#include "Transport.h"  // for DEFAULT_PRIORITY

namespace VLCB
{

const int LONG_MESSAGE_DEFAULT_DELAY = 20;      // delay in milliseconds between sending successive long message fragments
const int LONG_MESSAGE_RECEIVE_TIMEOUT = 5000;  // timeout waiting for next long message packet
const int NUM_EX_CONTEXTS = 4;                  // number of send and receive contexts for extended implementation = number of concurrent messages
const int EX_BUFFER_LEN = 64;                   // size of extended send and receive buffers

//
/// Controller long message status codes
//

enum {
  LONG_MESSAGE_INCOMPLETE = 0,
  LONG_MESSAGE_COMPLETE,
  LONG_MESSAGE_SEQUENCE_ERROR,
  LONG_MESSAGE_TIMEOUT_ERROR,
  LONG_MESSAGE_CRC_ERROR,
  LONG_MESSAGE_TRUNCATED
};

//
/// a basic class to send and receive Controller long messages per MERG RFC 0005
/// See https://www.merg.org.uk/merg_wiki/doku.php?id=rfc:longmessageprotocol
/// handles a single message, sending and receiving
/// suitable for small microcontrollers with limited memory
//
class LongMessageService : public Service
{

public:

  virtual void setController(Controller *cntrl) override { this->controller = cntrl; }
  bool sendLongMessage(const void *msg, const unsigned int msg_len, const byte stream_id, const byte priority = DEFAULT_PRIORITY);
  void subscribe(byte *stream_ids, const byte num_stream_ids, void *receive_buffer, const unsigned int receive_buffer_len, void (*messagehandler)(void *fragment, const unsigned int fragment_len, const byte stream_id, const byte status));
  Processed handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID) override;
  bool process();
  virtual void processReceivedMessageFragment(const CANFrame *frame);
  bool is_sending();
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
  Controller *controller;
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
class LongMessageServiceEx : public LongMessageService
{
public:

  bool allocateContexts(byte num_receive_contexts = NUM_EX_CONTEXTS, unsigned int receive_buffer_len = EX_BUFFER_LEN, byte num_send_contexts = NUM_EX_CONTEXTS);
  bool sendLongMessage(const void *msg, const unsigned int msg_len, const byte stream_id, const byte priority = DEFAULT_PRIORITY);
  bool process();
  void subscribe(byte *stream_ids, const byte num_stream_ids, void (*messagehandler)(void *msg, unsigned int msg_len, byte stream_id, byte status));
  virtual void processReceivedMessageFragment(const CANFrame *frame);
  byte is_sending();
  void use_crc(bool use_crc);

private:

  bool _use_crc = false;
  byte _num_receive_contexts = NUM_EX_CONTEXTS, _num_send_contexts = NUM_EX_CONTEXTS;
  receive_context_t **_receive_context = NULL;
  send_context_t **_send_context = NULL;
};

}