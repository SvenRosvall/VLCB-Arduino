
#include <iostream>
#include <vector>
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



void testEncodeStandardMessage_ID(int ID, const char * expectedMessage, bool expectedResult){
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


void testEncodeExtendedMessage_ID(int ID, const char * expectedMessage, bool expectedResult){
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


void testEncodeMessage_RTR(bool rtr, const char * expectedMessage, bool expectedResult){
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

void testEncodeMessage_DATA(int len, const char * expectedMessage, bool expectedResult){
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





void testDecodeStandardIdMessage(){

}

void testExtendedIdMessage(){
  
}

void testGridConnect(void){

  // test standard ID - 11 bits, max id 0x7FF
  testEncodeStandardMessage_ID(0x0, ":S0000N0102;", true);
  testEncodeStandardMessage_ID(0x1, ":S0020N0102;", true);
  testEncodeStandardMessage_ID(0x7FF, ":SFFE0N0102;", true);
  testEncodeStandardMessage_ID(0x800, "", false);
  testEncodeStandardMessage_ID(0xFFFF, "", false);

  // test extended ID - 29 bits, max Id 0x1FFFFFFF
  // encoded with bits 18 to 28 shifted by 3
  // so need to test around this gap
  testEncodeExtendedMessage_ID(0x0, ":X00000000N0102;", true);        // all bits zero
  testEncodeExtendedMessage_ID(0x1, ":X00000001N0102;", true);        // just bit 0 set
  testEncodeExtendedMessage_ID(0x20000, ":X00020000N0102;", true);    // bit 17 - before break in coding
  testEncodeExtendedMessage_ID(0x3FFFF, ":X0003FFFFN0102;", true);    // all up to bit 17 - 18 bits
  testEncodeExtendedMessage_ID(0x40000, ":X00200000N0102;", true);    // bit 18 - after break in coding
  testEncodeExtendedMessage_ID(0x1FFC0000, ":XFFE00000N0102;", true); // bits 18 to 28 - 11 bits
  testEncodeExtendedMessage_ID(0x1FFFFFFF, ":XFFE3FFFFN0102;", true); // all 29 bits set
  testEncodeExtendedMessage_ID(0x20000000, "", false);
  testEncodeExtendedMessage_ID(0xFFFFFFFF, "", false);


  // test RTR character (boolean, so no invalid value)
  testEncodeMessage_RTR(false, ":SFFE0N0102;", true);
  testEncodeMessage_RTR(true, ":SFFE0R0102;", true);

  // test data field, passing length parameter to the test
  testEncodeMessage_DATA(0, ":SFFE0N;", true);
  testEncodeMessage_DATA(1, ":SFFE0N01;", true);
  testEncodeMessage_DATA(8, ":SFFE0N0102030405060708;", true);
  testEncodeMessage_DATA(9, "", false);


/*
  char * txBuffer;
  CANMessage *msg;
  msg->ext = 0;
  msg->len = 2;
  msg->rtr = 0;
  msg->id = 0x07FF;
  msg->data[0] = 1;
  msg->data[1] = 2;
  bool result = VLCB::encodeGridConnect(txBuffer, msg);

  std::cout << "result " << result << std::endl;
  std::cout << "txBuffer " << txBuffer << std::endl;

  CANMessage *rxMsg;
  result = VLCB::decodeGridConnect(txBuffer, rxMsg);

  debugCANMessage(*rxMsg);
*/

}