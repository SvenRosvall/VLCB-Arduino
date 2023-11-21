
#include <iostream>
#include "TestTools.hpp"

#include "GridConnect.h"

void debugCANMessage(CANMessage message)
  {
    std::cout << std::endl << "CANMessage:";
    std::cout << " id " << message.id << " length " << message.len;
    std::cout << " data ";
    for (int i=0; i <message.len; i++) {
      if( i>0 ) std::cout << ",";
      std::cout << message.data[i];
    }
    std::cout << std::endl;
  }



void testGridConnectEncode_StandardID(int ID, const char * expectedMessage, bool expectedResult){
  test();
  char msgBuffer[28]; 
  CANMessage msg;
  msg.ext = 0;
  msg.len = 2;
  msg.rtr = 0;
  msg.id = ID;
  msg.data[0] = 1;
  msg.data[1] = 2;

  bool result = VLCB::encodeGridConnect(msgBuffer, &msg);

  assertEquals(expectedMessage, msgBuffer);
  assertEquals(expectedResult, result);

//  std::cout << "encoded message " << msgBuffer << std::endl;  
}


void testGridConnectEncode_ExtendedID(int ID, const char * expectedMessage, bool expectedResult){
  test();
  char msgBuffer[28]; 
  CANMessage msg;
  msg.ext = true;
  msg.len = 2;
  msg.rtr = 0;
  msg.id = ID;
  msg.data[0] = 1;
  msg.data[1] = 2;

  bool result = VLCB::encodeGridConnect(msgBuffer, &msg);

  assertEquals(expectedMessage, msgBuffer);
  assertEquals(expectedResult, result);

//  std::cout << "encoded message " << msgBuffer << std::endl;  
}


void testGridConnectEncode_RTR(bool rtr, const char * expectedMessage, bool expectedResult){
  test();
  char msgBuffer[28]; 
  CANMessage msg;
  msg.ext = 0;
  msg.len = 2;
  msg.rtr = rtr;
  msg.id = 0x7FF;
  msg.data[0] = 1;
  msg.data[1] = 2;

  bool result = VLCB::encodeGridConnect(msgBuffer, &msg);

  assertEquals(expectedMessage, msgBuffer);
  assertEquals(expectedResult, result);

//  std::cout << "encoded message " << msgBuffer << std::endl;  
}

void testGridConnectEncode_DATA(int len, const char * expectedMessage, bool expectedResult){
  test();
  char msgBuffer[30]; 
  CANMessage msg;
  msg.ext = false;
  msg.len = len;
  msg.rtr = false;
  msg.id = 0x7FF;
  for (int i = 0; i<8; i++){
    msg.data[i] = i+1;
  }

  bool result = VLCB::encodeGridConnect(msgBuffer, &msg);

  assertEquals(expectedMessage, msgBuffer);
  assertEquals(expectedResult, result);

  //std::cout << "encoded message " << msgBuffer << std::endl << std::endl;  
}


void testGridConnectDecode_ID(const char * inputMessage, bool expectedEXT, int expectedID, bool expectedResult)
{
  test();
  CANMessage msg;
  bool result = VLCB::decodeGridConnect(inputMessage, &msg);

  assertEquals(expectedResult, result);
  if (result)
  { // only check ID if result is true (i.e. decode worked)
    assertEquals(expectedEXT, msg.ext);
    assertEquals(expectedID, msg.id);
    //std::cout << "decoded ID " << msg.id << std::endl << std::endl;  
  }
}

void testGridConnectDecode_RTR(const char * inputMessage, bool expectedRTR, bool expectedResult)
{
  test();
  CANMessage msg;
  bool result = VLCB::decodeGridConnect(inputMessage, &msg);

  assertEquals(expectedResult, result);
  if (result)
  { // only check ID if result is true (i.e. decode worked)
    assertEquals(expectedRTR, msg.rtr);
  }
}

void testGridConnectDecode_DATA(const char * inputMessage, int expectedLEN, bool expectedResult)
{
  test();
  CANMessage msg;
  bool result = VLCB::decodeGridConnect(inputMessage, &msg);

  assertEquals(expectedResult, result);
  if (result)
  { // only check ID if result is true (i.e. decode worked)
    assertEquals(expectedLEN, msg.len);
  }
}

void testGridConnect(void){

  // test encoding standard ID - 11 bits, max id 0x7FF
  testGridConnectEncode_StandardID(0x0, ":S0000N0102;", true);
  testGridConnectEncode_StandardID(0x1, ":S0020N0102;", true);
  testGridConnectEncode_StandardID(0x7FF, ":SFFE0N0102;", true);
  testGridConnectEncode_StandardID(0x800, "", false);
  testGridConnectEncode_StandardID(0xFFFF, "", false);

  // test encoding extended ID - 29 bits, max Id 0x1FFFFFFF
  // encoded with bits 18 to 28 shifted by 3
  // so need to test around this gap
  testGridConnectEncode_ExtendedID(0x0, ":X00000000N0102;", true);        // all bits zero
  testGridConnectEncode_ExtendedID(0x1, ":X00000001N0102;", true);        // just bit 0 set
  testGridConnectEncode_ExtendedID(0x20000, ":X00020000N0102;", true);    // bit 17 - before break in coding
  testGridConnectEncode_ExtendedID(0x3FFFF, ":X0003FFFFN0102;", true);    // all up to bit 17 - 18 bits
  testGridConnectEncode_ExtendedID(0x40000, ":X00200000N0102;", true);    // bit 18 - after break in coding
  testGridConnectEncode_ExtendedID(0x1FFC0000, ":XFFE00000N0102;", true); // bits 18 to 28 - 11 bits
  testGridConnectEncode_ExtendedID(0x1FFFFFFF, ":XFFE3FFFFN0102;", true); // all 29 bits set
  testGridConnectEncode_ExtendedID(0x20000000, "", false);
  testGridConnectEncode_ExtendedID(0xFFFFFFFF, "", false);

  // test encoding RTR character (boolean, so no invalid value)
  testGridConnectEncode_RTR(false, ":SFFE0N0102;", true);
  testGridConnectEncode_RTR(true, ":SFFE0R0102;", true);

  // test encoding data field, passing length parameter to the test
  testGridConnectEncode_DATA(0, ":SFFE0N;", true);
  testGridConnectEncode_DATA(1, ":SFFE0N01;", true);
  testGridConnectEncode_DATA(8, ":SFFE0N0102030405060708;", true);
  testGridConnectEncode_DATA(9, "", false);

  // test decoding ID field, expectedEXT is false
  testGridConnectDecode_ID(":S0000N;", false, 0x0, true);             // all bits clear
  testGridConnectDecode_ID(":S0020N;", false, 0x1, true);             // bit 1 set
  testGridConnectDecode_ID(":SFFE0N;", false, 0x7FF, true);           // bits 0 to 10 set
  testGridConnectDecode_ID(":SQFE0N;", false, 0x0, false);            // non hex char in ID
  testGridConnectDecode_ID(":SFFEQN;", false, 0x0, false);            // non hex char in ID

  // test decoding extended ID field, expectedEXT is true
  testGridConnectDecode_ID(":X00000000N;", true, 0x0, true);          // all bits clear
  testGridConnectDecode_ID(":X00000001N;", true, 0x1, true);          // bit 1 set
  testGridConnectDecode_ID(":X00020000N;", true, 0x20000, true);      // bit 17 set
  testGridConnectDecode_ID(":X0003FFFFN;", true, 0x3FFFF, true);      // bits 0 to 17 set
  testGridConnectDecode_ID(":X00200000N;", true, 0x40000, true);      // bit 18 set
  testGridConnectDecode_ID(":XFFE00000N;", true, 0x1FFC0000, true);   // bits 18 to 28 set
  testGridConnectDecode_ID(":XFFE3FFFFN;", true, 0x1FFFFFFF, true);   // all bits 0 to 28 set
  testGridConnectDecode_ID(":XFFFFFFFFN;", true, 0x1FFFFFFF, true);   // check it ignores unused bits
  testGridConnectDecode_ID(":XQ0000000N;", true, 0x0, false);         // non hex char in ID
  testGridConnectDecode_ID(":X0000000QN;", true, 0x0, false);         // non hex char in ID
  
  // test decode RTR - params are inoput msg, expected RTR, expected result from decode
  testGridConnectDecode_RTR(":S0000N;", false, true);             // standard msg, RTR false
  testGridConnectDecode_RTR(":S0000R;", true, true);              // standard msg, RTR true
  testGridConnectDecode_RTR(":S0000Q;", true, false);             // standard msg, invalid char in RTR field
  testGridConnectDecode_RTR(":X00000000N;", false, true);         // extended msg, RTR false
  testGridConnectDecode_RTR(":X00000000R;", true, true);          // extended msg, RTR true
  testGridConnectDecode_RTR(":X00000000Q;", true, false);          // extended msg, invalid char in RTR field

  // test decode data field, standard message
  testGridConnectDecode_DATA(":S0000N;", 0, true);                          // standard msg, zero data
  testGridConnectDecode_DATA(":S0000N01;", 1, true);                        // standard msg, 1 data byte
  testGridConnectDecode_DATA(":S0000N0102030405060708;", 8, true);          // standard msg, 8 data bytes
  testGridConnectDecode_DATA(":S0000N0;", 1, false);                        // standard msg, wrong number of data chars
  testGridConnectDecode_DATA(":S0000N010203040506070;", 8, false);          // standard msg, wrong number of data chars
  testGridConnectDecode_DATA(":S0000N010203040506070809;", 9, false);       // standard msg, too many data bytes
  testGridConnectDecode_DATA(":S0000NQ102030405060708;", 8, false);          // standard msg, invalid char in data
  testGridConnectDecode_DATA(":S0000N010203040506070Q;", 8, false);          // standard msg, invalid char in data

  // test decode data field, extended message
  testGridConnectDecode_DATA(":X00000000N;", 0, true);                      // extended msg, zero data
  testGridConnectDecode_DATA(":X00000000N01;", 1, true);                    // extended msg, 1 data byte
  testGridConnectDecode_DATA(":X00000000N0102030405060708;", 8, true);      // extended msg, 8 data bytes
  testGridConnectDecode_DATA(":X00000000N0;", 1, false);                    // extended msg, wrong number of data chars
  testGridConnectDecode_DATA(":X00000000N010203040506070;", 8, false);      // extended msg, wrong number of data chars
  testGridConnectDecode_DATA(":X00000000N010203040506070809;", 9, false);   // extended msg, too many data bytes
  testGridConnectDecode_DATA(":X00000000NQ102030405060708;", 8, false);     // extended msg, invalid char in data
  testGridConnectDecode_DATA(":X00000000N010203040506070Q;", 8, false);     // extended msg, invalid char in data


}