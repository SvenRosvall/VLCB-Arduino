//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Limited testing of Controller class.
// Only created to fill gaps in code coverage testing.

#include "Controller.h"
#include "MockTransportService.h"
#include "TestTools.hpp"
#include "VlcbCommon.h"

namespace
{
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController()
{
  mockTransportService.reset(new MockTransportService);

  VLCB::Controller controller = ::createController({mockTransportService.get()});
  controller.begin();

  return controller;
}

void testSendMessage0()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(1, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
}

void testSendMessage1()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(2, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
}

void testSendMessage2()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(3, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
}

void testSendMessage3()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102, 103);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(4, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
  assertEquals(103, mockTransportService->sent_messages[0].data[3]);
}

void testSendMessage4()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102, 103, 104);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
  assertEquals(103, mockTransportService->sent_messages[0].data[3]);
  assertEquals(104, mockTransportService->sent_messages[0].data[4]);
}

void testSendMessage5()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102, 103, 104, 105);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(6, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
  assertEquals(103, mockTransportService->sent_messages[0].data[3]);
  assertEquals(104, mockTransportService->sent_messages[0].data[4]);
  assertEquals(105, mockTransportService->sent_messages[0].data[5]);
}

void testSendMessage6()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102, 103, 104, 105, 106);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(7, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
  assertEquals(103, mockTransportService->sent_messages[0].data[3]);
  assertEquals(104, mockTransportService->sent_messages[0].data[4]);
  assertEquals(105, mockTransportService->sent_messages[0].data[5]);
  assertEquals(106, mockTransportService->sent_messages[0].data[6]);
}

void testSendMessage7()
{
  test();
  
  VLCB::Controller controller = createController();
  
  controller.sendMessage(OPC_TON, 101, 102, 103, 104, 105, 106, 107);
  
  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(8, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_TON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(101, mockTransportService->sent_messages[0].data[1]);
  assertEquals(102, mockTransportService->sent_messages[0].data[2]);
  assertEquals(103, mockTransportService->sent_messages[0].data[3]);
  assertEquals(104, mockTransportService->sent_messages[0].data[4]);
  assertEquals(105, mockTransportService->sent_messages[0].data[5]);
  assertEquals(106, mockTransportService->sent_messages[0].data[6]);
  assertEquals(107, mockTransportService->sent_messages[0].data[7]);
}

}

void testController()
{
  testSendMessage0();
  testSendMessage1();
  testSendMessage2();
  testSendMessage3();
  testSendMessage4();
  testSendMessage5();
  testSendMessage6();
  testSendMessage7();

}