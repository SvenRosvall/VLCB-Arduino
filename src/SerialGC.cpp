// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <SerialGC.h>
// 3rd party libraries
#include <Streaming.h>


namespace VLCB
{

  //
  /// constructor
  //
  SerialGC::SerialGC()  { }


   void SerialGC::begin()  {
    Serial << F("> ** GridConnect over serial ** ") << endl;
 }

  //
  /// check for unprocessed messages in the buffer
  //
  bool SerialGC::available()
  {
    return true;
  }



  //
  /// get next unprocessed message from the buffer
  /// must call available first to ensure there is something to get
  //
  CANMessage SerialGC::getNextCanMessage()
  {
    CANMessage message;       // ACAN2515 frame class

    return message;
  }

  //
  /// send a VLCB message
  //
  bool SerialGC::sendCanMessage(CANMessage *msg)
  {
    // Gridconnect format of a CAN message with standard Identifier, in hexadecimal
    // :SBBBBCDDDDDDDDDDDDDDDD;
    // 012345678901234567890123 - 24 characters maximum
    // where 'S' represents standard
    // and for extended identier
    // :XBBBBBBBBCDDDDDDDDDDDDDDDD;
    // 0123456789012345678901234567 - 28 characters maximum
    // where 'X' represents extended
    // in both, 
    // B is the CAN identifier
    // C is 'R' for RTR or 'N' for normal
    // D are databytes, variable length 0 to 16 (in hexadecimal pairs)

    // start char array for output string
    char str[30];
    byte offset = 0;
    // set starting character & standard or extended CAN identifier
    if (msg->ext) {
      strcpy (str,":X");
      // extended CAN idenfier in bytes 2 to 9
      sprintf(str + 2, "%08X", msg->id);
      offset = 10;
    } else {
      strcpy (str,":S");
      // standard CAN idenfier in bytes 2 to 5
      sprintf(str + 2, "%04X", msg->id);
      offset = 6;
    }
    // set RTR or normal - byte 6 or 10
    if (msg->rtr) {
      strcpy (str + offset++,"R");
    } else {
      strcpy (str + offset++,"N");
    }
    // add terminator in case len = 0, will be overwritten if len >0
    strcpy (str + offset,";");
    //now data from byte 7 if len > 0
    for (int i=0; i<msg->len; i++){
      sprintf(str + offset + i*2, "%02X", msg->data[i]);
      // append terminator after every data byte - will be overwritten except for last one
      strcpy (str + offset + 2 + i*2,";");
    }
    // output the message
    Serial << str << endl;
    return true;
  }

  //
  /// reset
  //
  void SerialGC::reset()
  {
  }

}