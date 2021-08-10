
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)
  Based on MERGG RFC 0005 by Dave McCabe (M4933)

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

//
/// Implementation of CBUS Long Message Support RFC 0005 by Dave McCabe (M4933)
/// Developed by Duncan Greenwood (M5767)
//

#include <CBUS.h>
#include <Streaming.h>

uint32_t crc32(const char *s, size_t n);

//
/// constructor
/// receives a reference to a CBUS object which provides the message sending capability
//

CBUSLongMessage::CBUSLongMessage(CBUSbase *cbus_object_ptr) {

	_cbus_object_ptr = cbus_object_ptr;
	_send_priority = DEFAULT_PRIORITY;
	_msg_delay = LONG_MESSAGE_DEFAULT_DELAY;
	_receive_timeout = LONG_MESSAGE_RECEIVE_TIMEOUT;
	_cbus_object_ptr->setLongMessageHandler(this);
}

//
/// subscribe to a range of stream IDs
//

void CBUSLongMessage::subscribe(byte *stream_ids, const byte num_stream_ids, byte *receive_buffer, const unsigned int receive_buff_len, void (*messagehandler)(byte *msg, unsigned int msg_len, byte stream_id, byte status)) {

	_stream_ids = stream_ids;
	_num_stream_ids = num_stream_ids;
	_receive_buffer = receive_buffer;
	_receive_buff_len = receive_buff_len;
	_messagehandler = messagehandler;

	Serial << F("> subscribe: num_stream_ids = ") << num_stream_ids << F(", receive_buff_len = ") << receive_buff_len << endl;
	return;
}

//
/// initiate sending of a long message
/// this method sends the first message - the header packet
/// the remainder of the message is sent in chunks from the process() method
//

bool CBUSLongMessage::sendLongMessage(const byte *msg, const unsigned int msg_len, const byte stream_id, const byte priority) {

	CANFrame frame;

	// Serial << F("> L: sending message header packet, stream id = ") << stream_id << F(", message length = ") << msg_len << F(", first char = ") << (char)msg[0] << endl;

	if (is_sending()) {
		Serial << F("> L: ERROR: there is already a message is progress") << endl;
		return false;
	}

	// initialise variables
	_send_buffer = (byte *)msg;
	_send_buffer_len = msg_len;
	_send_stream_id = stream_id;
	_send_priority = priority;
	_send_buffer_index = 0;
	_send_sequence_num = 0;

	// send the first fragment which forms the message header
	frame.data[1] = _send_stream_id;																									// the unique stream id
	frame.data[2] = _send_sequence_num;																								// sequence number, 0 = header packet
	frame.data[3] = highByte(msg_len);																								// the message length
	frame.data[4] = lowByte(msg_len);
	frame.data[5] = 0;																																// CRC - not currently implemented
	frame.data[6] = 0;
	frame.data[7] = 0;																																// flags - not currently used

	bool ret = sendMessageFragment(&frame, _send_priority);														// send the header packet
	++_send_sequence_num;																															// increment the sending sequence number - it's fine if it wraps around

	// Serial << F("> L: message header sent, stream id = ") << _send_stream_id << F(", message length = ") << _send_buffer_len << endl;
	return (ret);
}

//
/// the process method is called regularly from the main CBUS loop
/// we use this to send the individual fragments of an outgoing message
//

void CBUSLongMessage::process(void) {

	byte i;
	CANFrame frame;

	/// check receive timeout

	if (_is_receiving && (millis() - _last_fragment_received >= _receive_timeout)) {
		Serial << F("> L: ERROR: timed out waiting for continuation packet") << endl;
		(void)(*_messagehandler)(_receive_buffer, _receive_buffer_index, _receive_stream_id, CBUS_LONG_MESSAGE_TIMEOUT_ERROR);
		_is_receiving = false;
		_incoming_message_length = 0;
		_incoming_bytes_received = 0;
	}

	/// send the next outgoing fragment, after a configurable delay to avoid flooding the bus

	if (_send_buffer_index < _send_buffer_len && (millis() - _last_fragment_sent >= _msg_delay)) {

		_last_fragment_sent = millis();

		memset(&frame.data, 0, sizeof(frame.data));
		frame.data[1] = _send_stream_id;
		frame.data[2] = _send_sequence_num++;

		/// only the last fragment is potentially less than 5 bytes long

		for (i = 0; i < 5 && _send_buffer_index < _send_buffer_len; i++) {								// for up to 5 bytes of payload
			frame.data[i + 3] = _send_buffer[_send_buffer_index];														// take the next byte
			// Serial << F("> L: consumed data byte = ") << (char)_send_buffer[_send_buffer_index] << endl;
			++_send_buffer_index;
		}

		sendMessageFragment(&frame, _send_priority);																			// send the data packet
		// Serial << F("> L: process: sent message fragment, size = ") << i << F(", send buffer len = ") << _send_buffer_len << F(", index now = ") << _send_buffer_index << endl;
	}

	return;
}

//
/// handle a received long message fragment
/// this method is called by the main CBUS object each time a long CBUS message is received (opcode 0xe9)
//

void CBUSLongMessage::processReceivedMessageFragment(const CANFrame *frame) {

	/// handle a received message fragment

	Serial << F("> L: processing received long message packet, message length = ") << _incoming_message_length << F(", rcvd so far = ") << _incoming_bytes_received << endl;

	_last_fragment_received = millis();

	byte i, j;

	if (!_is_receiving) {																																// not currently receiving a long message

		if (frame->data[2] == 0) {																												// sequence zero = a header packet with start of new stream
			for (i = 0; i < _num_stream_ids; i++) {
				if (_stream_ids[i] == frame->data[1]) {																				// are we subscribed to this stream id ?
					_is_receiving = true;
					_receive_stream_id = frame->data[1];
					_incoming_message_length = (frame->data[3] << 8) + frame->data[4];
					_incoming_message_crc = (frame->data[5] << 8) + frame->data[6];
					_incoming_bytes_received = 0;
					memset(_receive_buffer, 0, _receive_buff_len);
					_receive_buffer_index = 0;
					_expected_next_receive_sequence_num = 0;
					Serial << F("> L: received header packet for stream id = ") << _receive_stream_id << F(", message length = ") << _incoming_message_length << F(", user buffer len = ") << _receive_buff_len << endl;
					break;
				}
			}
		}

	} else {																																						// we're part way through receiving a message

		if (frame->data[1] == _receive_stream_id) {																				// it's the same stream id

			if (frame->data[2] == _expected_next_receive_sequence_num) {										// and it's the expected sequence id

				Serial << F("> L: received continuation packet, seq = ") << _expected_next_receive_sequence_num << endl;

				// for each of the maximum five payload bytes, up to the total message length and the user buffer length
				for (j = 0; j < 5; j++) {

					_receive_buffer[_receive_buffer_index] = frame->data[j + 3];								// take the next byte
					++_receive_buffer_index;																										// increment the buffer index
					++_incoming_bytes_received;
					// Serial << F("> L: processing received data byte = ") << (char)frame->data[j + 3] << endl;

					// if we have read the entire message
					if (_incoming_bytes_received >= _incoming_message_length) {
						Serial << F("> L: bytes processed = ") << _incoming_bytes_received << F(", message data has been fully consumed") << endl;
						(void)(*_messagehandler)(_receive_buffer, _receive_buffer_index, _receive_stream_id, CBUS_LONG_MESSAGE_COMPLETE);
						_receive_buffer_index = 0;
						memset(_receive_buffer, 0, _receive_buff_len);
						break;

						// if the user buffer is full
					} else if (_receive_buffer_index >= _receive_buff_len ) {
						Serial << F("> L: user buffer is full") << endl;
						(void)(*_messagehandler)(_receive_buffer, _receive_buffer_index, _receive_stream_id, CBUS_LONG_MESSAGE_INCOMPLETE);
						_receive_buffer_index = 0;
						memset(_receive_buffer, 0, _receive_buff_len);
					}
				}

			} else {																																				// it's the wrong sequence id
				Serial << F("> L: ERROR: expected receive sequence num = ") << _expected_next_receive_sequence_num << F(" but got = ") << frame->data[2] << endl;
				(void)(*_messagehandler)(_receive_buffer, _receive_buffer_index, _receive_stream_id, CBUS_LONG_MESSAGE_SEQUENCE_ERROR);
				_incoming_message_length = 0;
				_incoming_bytes_received = 0;
				_is_receiving = false;
			}

		} else {																																					// probably another stream in progress - we'll ignore it as we don't support concurrent streams
			Serial << F("> L: ignoring unexpected stream id =") << _receive_stream_id << F(", got = ") << frame->data[2] << endl;
		}

	}	// it's a continuation fragment

	// the sequence number may wrap from 255 to 0, which is absolutely fine
	++_expected_next_receive_sequence_num;

	/// finally, once the message has been completely received ...

	if (_incoming_message_length > 0 && _incoming_bytes_received >= _incoming_message_length) {
		Serial << F("> L: message is complete") << endl;

		// surface any final fragment to the user's code
		if (_receive_buffer_index > 0) {
			Serial << F("> L: surfacing final fragment") << endl;
			(void)(*_messagehandler)(_receive_buffer, _receive_buffer_index, _receive_stream_id, CBUS_LONG_MESSAGE_COMPLETE);
		}

		// get ready for the next long message
		_incoming_message_length = 0;
		_incoming_bytes_received = 0;
		_receive_buffer_index = 0;
		_is_receiving = false;
	}

	return;
}

//
/// report progress of sending last long message
/// true = complete, false = in progress, incomplete
/// user code must not start a new message until the previous one has been completely sent
//

bool CBUSLongMessage::is_sending(void) {

	return (_send_buffer_index < _send_buffer_len);
}

//
/// send next message fragment
//

bool CBUSLongMessage::sendMessageFragment(CANFrame * frame, const byte priority) {

	// these are common to all messages
	frame->len = 8;
	frame->data[0] = OPC_LMSG;

	return (_cbus_object_ptr->sendMessage(frame, false, false, priority));
}

void CBUSLongMessage::setDelay(byte delay_in_millis) {

	_msg_delay = delay_in_millis;
	return;
}

//
/// set the delay between send fragments
/// overrides the default value
//

void CBUSLongMessage::setTimeout(unsigned int timeout_in_millis) {

	_receive_timeout = timeout_in_millis;
	return;
}

//
/// calculate CRC32 for the given message
//

uint32_t crc32(const char *s, size_t n) {

	uint32_t crc = 0xFFFFFFFF;

	Serial << F("> calculating crc32 for msg = |") << s << F("|") << endl;

	for (size_t i = 0; i < n; i++) {
		char ch = s[i];
		for (size_t j = 0; j < 8; j++) {
			uint32_t b = (ch ^ crc) & 1;
			crc >>= 1;
			if (b) crc = crc ^ 0xEDB88320;
			ch >>= 1;
		}
	}

	Serial << F("> crc32 = ") << crc << endl;

	return ~crc;
}

