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

VLCB::Controller createController()
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransportService.reset(new MockTransportService);

  eventProducerService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), eventProducerService.get(), mockTransportService.get()});
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
  byte nnen[] = { 0x01, 0x04, 0x00, 0x01};
  controller.getModuleConfig()->writeEvent(0, nnen);
  controller.getModuleConfig()->writeEventEV(0, 1, 1);

  eventProducerService->sendEvent(true, 1);

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
  byte nnen[] = { 0x01, 0x04, 0x00, 0x01};
  controller.getModuleConfig()->writeEvent(0, nnen);
  controller.getModuleConfig()->writeEventEV(0, 1, 1);

  eventProducerService->sendEvent(false, 1, 42);

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
  byte nnen[] = { 0x00, 0x00, 0x00, 0x05};
  controller.getModuleConfig()->writeEvent(0, nnen);
  controller.getModuleConfig()->writeEventEV(0, 1, 7);

  eventProducerService->sendEvent(true, 7, 42, 17);

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
  byte nnen[] = { 0x00, 0x00, 0x00, 0x05};
  controller.getModuleConfig()->writeEvent(0, nnen);
  controller.getModuleConfig()->writeEventEV(0, 1, 7);

  eventProducerService->sendEvent(false, 7, 42, 17, 234);

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

void testSetProducedDefaultEventsOnNewBoard()
{
  test();

  VLCB::Controller controller = createController();

  // Set module into Uninitialized mode to get unitialized memory.
  minimumNodeService->setUninitialised();

  controller.begin();

  minimumNodeService->setNormal(0x0104);

  // Should have no events configured at start
  assertEquals(0, controller.getModuleConfig()->numEvents());

  process(controller);

  assertEquals(1, controller.getModuleConfig()->numEvents());
  byte eventArray[VLCB::EE_HASH_BYTES];
  controller.getModuleConfig()->readEvent(0, eventArray);
  assertEquals(0x01, eventArray[0]);
  assertEquals(0x04, eventArray[1]);
  assertEquals(0x00, eventArray[2]);
  assertEquals(0x01, eventArray[3]);
  assertEquals(1, controller.getModuleConfig()->getEventEVval(0, 1));
}

void testSendEventMissingInTable()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // No event exists in event table.
  eventProducerService->sendEvent(true, 1);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_ACON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
}

void testSendEventDeletedFromTable()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Create a default produced event.
  byte nnen[] = { 0x01, 0x04, 0x00, 0x01};
  controller.getModuleConfig()->writeEvent(0, nnen);
  controller.getModuleConfig()->writeEventEV(0, 1, 1);
  // And delete it.
  controller.getModuleConfig()->cleareventEEPROM(0);

  // Event with EV1=1 exists but its nn/en has been cleared.
  eventProducerService->sendEvent(true, 1);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(5, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_ACON, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[4]);
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
  byte eventData[] = {0x01, 0x04, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
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
  byte eventData[] = {0x00, 0x00, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
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
  byte eventData[] = {0x00, 0x00, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
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
  byte eventData[] = {0x00, 0x00, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ASRQ, 0x01, 0x05, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(0xFF, capturedIndex);
}

void testLongSendRequestResponse()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  byte eventData[] = {0x01, 0x04, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
  configuration->updateEvHashEntry(1);

  eventProducerService->sendRequestResponse(true, 1);

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

void testShortSendRequestResponse()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventProducerService->setRequestEventHandler(mockHandler);
  controller.begin();

  // Add some long events
  byte eventData[] = {0x00, 0x00, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);

  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
  configuration->updateEvHashEntry(1);

  eventProducerService->sendRequestResponse(false, 1);

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
  testSetProducedDefaultEventsOnNewBoard();
  testSendEventMissingInTable();
  testSendEventDeletedFromTable();
  testLongRequestStatus();
  testShortRequestStatusWithNN();
  testShortRequestStatusWithoutNN();
  testShortRequestStatusWithDifferentNN();
  testLongSendRequestResponse();
  testShortSendRequestResponse();
}
