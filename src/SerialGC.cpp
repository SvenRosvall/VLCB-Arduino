// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <SerialGC.h>
// 3rd party libraries
#include <Streaming.h>
#include <string.h>

//
// Class to transfer CAN frames using the GridConnect protocol over the serial port
//
// GridConnect is a format to encode a bit orientated CAN frame onto a byte orientated serial stream
// The CBUS developers guide describes a slightly modified form of GridConnect, which has been used to 
// enable communtion between computers and can adapters like the CANUSB4 module
// this module follows this convention

// the GridConnect message syntax for a normal message
// : <S | X> <IDENTIFIER> <N> <DATA-0> <DATA-1> … <DATA-7> ;
// where:
//     ':' and ';' are message start and end characters respectively
//     Identifier and data fields are treated as base-16 digits (hexadecimal).
//     'S' & 'X' characters indicates standard or extended CAN frames
//     'N' character indicates a normal (not RTR) message

// the GridConnect message syntax for a 'Request to Transmit' (RTR) message (there is no data field)
// : <S | X> <IDENTIFIER> <R> <LENGTH>;
// where:
//     ':' and ';' are message start and end characters respectively
//     Identifier field is treated as base-16 digits (hexadecimal).
//     'S' & 'X' characters indicates standard or extended CAN frames
//     'R' character indicates an RTR message
//     Length field is a single ASCII decimal character from ‘0’ to ‘8’ that specifies the length
//     of the message

// the difference in the GridConnect version used here is in the format of the Identifier field:
//
// For a standard can message with an 11 bit identifier, the GridConnect identifier field is 4 hex characters,
// i.e. 16 bits
// The CBUS implementation shifts the 11 bit ID by 5, i.e. fills the top bit, with the lower 5 bits being 0
// (This was done to map directly onto SIDH and SIDL registers in the PIC processor family)
// And the GridConnect Identifier field is also leading zero padded, so always 4 characters for a standard message
// 
// For an extended can message with an 29 bit identifier, the GridConnect identifier field is 8 hex characters,
// i.e. 32 bits
// In this case the mapping is a bit more complex
// IDENTIFIER[0,1] - bits [28:21] of the can identifier
// IDENTIFIER[2] - bits [20:18] of the can identifier, left shifted by 1, bottom bit set to zero
// IDENTIFIER[3] - bits [17:16] of the can identifier, bottom two bits, top two bits set to 0
// IDENTIFIER[4,5,6,7] - bits [17:16] of the can identifier
// (This maps directly onto SIDH, SIDL, EIDH and EIDL registers the PIC processor family)
// And the GridConnect Identifier field is also leading zero padded, so always 8 characters for an extended message
// 


namespace VLCB
{

  // Function to convert a pair of hexadecimal characters to a byte value
  //
  int ascii_pair_to_byte(const char *pair)
  {
      unsigned char* data = (unsigned char*)pair;
      int result;
      if (data[1] < 'A') { result = data[1] - '0'; }
      else { result = data[1] - 'A' + 10; }
      if (data[0] < 'A') { result += (data[0] - '0') << 4; }
      else { result += (data[0] - 'A' + 10) << 4; }
      return result;
  }

  // check supplied array is comprised of only hexadecimal characters
  //
  bool checkHexChars(char *charBuff, int count)
  {
    for (int i = 0 ; i< count; i++)
    {
      if (!isxdigit(charBuff[i]))
      {
        return false;
      }
    }
    return true;
  }

  // convert a gridconnect message to CANMessage object
  // see Gridconnect format at beginning of file for byte positions
  //
  bool encodeCANMessage(char * gcBuffer, CANMessage *message) 
  {
    //Serial << " encode gridconnect message "<< gcBuffer << endl;

    int gcIndex = 0;                          // index used to 'walk' gc message
    int gcBufferLength = strlen(gcBuffer);    // save for later use

    // must have start of message character
    if (gcBuffer[gcIndex++] != ':') 
    {
      return false;
    }
    //
    // do CAN Identifier, must be either 'X' or 'S'
    if (gcBuffer[gcIndex] == 'X') 
    {
      message->ext = true;
      // now get 29 bit ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 8) == false)
      {
        return false;
      }
      message->id = strtol(&gcBuffer[2], NULL, 16);
      gcIndex = 10;
    }
    else if (gcBuffer[gcIndex] == 'S') 
    {
      message->ext = false;
      // now get 11 bit ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 4) == false)
      {
        return false;
      }
      // 11 bit identifier needs to be shifted right by 5
      message->id = strtol(&gcBuffer[2], NULL, 16) >> 5;
      gcIndex = 6;
    } 
    else 
    {
      return false;
    }
    //
    // do RTR flag
    if (gcBuffer[gcIndex] == 'R') 
    {
      message->rtr = true;
    } 
    else if (gcBuffer[gcIndex] == 'N')
    {
      message->rtr = false;
    } 
    else 
    {
      return false;
    }
    gcIndex++;  // set to next character afert RTR flag
    //
    // Do data segment - convert hex array to byte array
    // find out how many chars in data segment (0 to 16, in multiples of 2) 
    // should be gcBufferLength minus gcIndex as it is now (after RTR flag), minus 1
    int dataLength = gcBufferLength - gcIndex - 1;
    // must be even number of hex characters, and no more than 16
    if ((dataLength % 2 ) || (dataLength > 16 )) 
    {
      return false;
    } 
    else 
    {
      message->len = dataLength/2;
    }
    // now convert hex data into bytes
    for (int i = 0; i < dataLength/2; i++) 
    {
      // check they are hex chars first
      if (checkHexChars(&gcBuffer[gcIndex], 2) == false)
      {
        return false;
      }
      message->data[i] = ascii_pair_to_byte(&gcBuffer[gcIndex]);
      gcIndex += 2;
    }
    //
    // must have end of message character
    if (gcBuffer[gcBufferLength-1] != ';') 
    {
      return false;
    }
    //
    return true; 
  }

  // Function to output a debug CANMessage to Serial
  //
  void debugCANMessage(CANMessage message)
  {
    Serial << endl << "CANMessage:";
    Serial << " id " << message.id << " length " << message.len;
    Serial << " data ";
    for (int i=0; i <message.len; i++) {
      if( i>0 ) Serial << ",";
      Serial << message.data[i];
    }
    Serial << endl;
  }


  bool SerialGC::begin()  
  {
    Serial << F("> ** GridConnect over serial ** ") << endl;
    return true;
  }


  //
  // parse incoming characters & assemble message
  // return true if valid message assembled & ready
  //
  bool SerialGC::available()
  {
    bool result = false;
    static int rxIndex = 0;
    if (Serial.available())
    {     
      char c = Serial.read();
      if(c >= 'a' && c <= 'z') bitClear(c,5);   // ensure letters are upper case
      //
      // if 'start of message' already seen, save the character, and check for 'end of message'
      if (rxIndex > 0) 
      {
        rxBuffer[rxIndex++] = c;
        // check if end of buffer reached, and restart if so
        if (rxIndex >= RXBUFFERSIZE) 
        {
          rxIndex = 0;
        }
        //
        // check for 'end of message'
        if (c == ';')
        {
          rxBuffer[rxIndex++] = '\0';     // null terminate
          rxIndex = 0;
          result = true;
        }
      }
      //
      // always check for 'start of message'
      if (c == ':') 
      {
        rxIndex = 0;                    // restart at beginning of buffer
        rxBuffer[rxIndex++] = c;
      }
    }
    //
    if (result) 
    {
      result = encodeCANMessage(rxBuffer, &rxCANMessage);
    }
    //
    return result;
  }


  //
  /// get the available CANMessage
  /// must call available first to ensure there is something to get
  //
  CANMessage SerialGC::getNextCanMessage()
  {
    debugCANMessage(rxCANMessage);
    return rxCANMessage;
  }

  //
  /// send a CANMessage message in GridConnect format
  // see Gridconnect format at beginning of file for byte positions
  //
  bool SerialGC::sendCanMessage(CANMessage *msg)
  {
    static int x = 0;
    // start char array for output string
    char str[30];
    byte offset = 0;
    // set starting character & standard or extended CAN identifier
    if (msg->ext) {
      strcpy (str,":X");
      // extended 29 bit CAN idenfier in bytes 2 to 9
      sprintf(str + 2, "%08X", msg->id);
      offset = 10;
    } else {
      strcpy (str,":S");
      // standard 11 bit CAN idenfier in bytes 2 to 5, left shifted 5 to occupy highest bits
      sprintf(str + 2, "%04X", msg->id << 5);
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
    Serial << endl << x++ << " ";
    // output the message
    Serial.print(str);
   return true;
  }

  //
  /// reset
  //
  void SerialGC::reset()
  {
  }


}