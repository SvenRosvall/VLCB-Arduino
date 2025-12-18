//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for EventProducerService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "EventProducerService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "MockTransportService.h"

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
std::unique_ptr<VLCB::EventProducerService> eventProducerService;
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController(VlcbModeParams startupMode = MODE_NORMAL)
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransportService.reset(new MockTransportService);

  eventProducerService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller = ::createController(startupMode, {minimumNodeService.get(), eventProducerService.get(), mockTransportService.get()});
  controller.begin();

  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(4, mockTransportService->sent_messages.size());

  assertEquals(OPC_SD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(3, mockTransportService->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransportService->sent_messages[1].data[0]);
  assertEquals(1, mockTransportService->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransportService->sent_messages[2].data[0]);
  assertEquals(2, mockTransportService->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_PRODUCER, mockTransportService->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ESD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(2, mockTransportService->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_PRODUCER, mockTransportService->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testSendOn()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  controller.getModuleConfig()->writeEvent(0, 260, 1);
  controller.getModuleConfig()->writeEventEV(0, 1, 1);

  eventProducerService->sendEventAtIndex(true, 0);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_ACON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
}

void testSend1Off()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  controller.getModuleConfig()->writeEvent(0, 260, 1);
  controller.getModuleConfig()->writeEventEV(0, 1, 1);

  eventProducerService->sendEventAtIndex(false, 0, 42);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ACOF1, mockTransportService->sent_messages[0].data[0]);
  assertEquals(6, mockTransportService->sent_messages[0].len);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
  assertEquals(42, mockTransportService->sent_messages[0].data[5]);
}

void testSendShort2On()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  controller.getModuleConfig()->writeEvent(0, 0, 5);
  controller.getModuleConfig()->writeEventEV(0, 1, 7);

  eventProducerService->sendEventAtIndex(true, 0, 42, 17);

  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ASON2, mockTransportService->sent_messages[0].data[0]);
  assertEquals(7, mockTransportService->sent_messages[0].len);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[4]);
  assertEquals(42, mockTransportService->sent_messages[0].data[5]);
  assertEquals(17, mockTransportService->sent_messages[0].data[6]);
}

void testSendShort3Off()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  controller.getModuleConfig()->writeEvent(0, 0, 5);
  controller.getModuleConfig()->writeEventEV(0, 1, 7);

  eventProducerService->sendEventAtIndex(false, 0, 42, 17, 234);

  process(controller);
  
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ASOF3, mockTransportService->sent_messages[0].data[0]);
  assertEquals(8, mockTransportService->sent_messages[0].len);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[4]);
  assertEquals(42, mockTransportService->sent_messages[0].data[5]);
  assertEquals(17, mockTransportService->sent_messages[0].data[6]);
  assertEquals(234, mockTransportService->sent_messages[0].data[7]);
}

void testNoDefaultEventsOnNewBoard()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);

  controller.begin();

  minimumNodeService->setNormal(0x0104);

  process(controller);

  // Default events are not created automatically
  assertEquals(0, controller.getModuleConfig()->numEvents());
}

byte capturedIndex;
VLCB::VlcbMessage capturedMessage;

void mockHandler(byte index, const VLCB::VlcbMessage * msg)
{
  capturedIndex = index;
  capturedMessage = *msg;
}

void testLongRequestStatus()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 260, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 260, 1);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_AREQ, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_AREQ, capturedMessage.data[0]);
}

void testShortRequestStatusWithNN()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 0, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 0, 1);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ASRQ, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ASRQ, capturedMessage.data[0]);
}

void testShortRequestStatusWithoutNN()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 0, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 0, 1);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ASRQ, 0, 0, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ASRQ, capturedMessage.data[0]);
}

void testShortRequestStatusWithDifferentNN()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 0, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 0, 1);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ASRQ, 0x01, 0x05, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(0xFF, capturedIndex);
}

void testLongSendEventResponse()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 260, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 260, 1);
  configuration->updateEvHashEntry(1);

  eventProducerService->sendEventResponse(true, 1);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_ARON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
}

void testShortSendEventResponse()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  configuration->writeEvent(0, 0, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 0, 1);
  configuration->updateEvHashEntry(1);

  eventProducerService->sendEventResponse(false, 1);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_ARSOF, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
}

}

void testEventProducerService()
{
  testServiceDiscovery();
  testServiceDiscoveryEventProdSvc();
  testSendOn();
  testSend1Off();
  testSendShort2On();
  testSendShort3Off();
  testNoDefaultEventsOnNewBoard();
  testLongRequestStatus();
  testShortRequestStatusWithNN();
  testShortRequestStatusWithoutNN();
  testShortRequestStatusWithDifferentNN();
  testLongSendEventResponse();
  testShortSendEventResponse();
}
