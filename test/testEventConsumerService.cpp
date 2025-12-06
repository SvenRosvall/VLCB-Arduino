//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for EventConsumerService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "EventConsumerService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "MockTransportService.h"

namespace
{
std::unique_ptr<VLCB::EventConsumerService> eventConsumerService;
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransportService.reset(new MockTransportService);

  eventConsumerService.reset(new VLCB::EventConsumerService);

  VLCB::Controller controller = ::createController( {minimumNodeService.get(), eventConsumerService.get(), mockTransportService.get()});
  controller.begin();

  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

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
  assertEquals(SERVICE_ID_CONSUMER, mockTransportService->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ESD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(2, mockTransportService->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_CONSUMER, mockTransportService->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

byte capturedIndex;
VLCB::VlcbMessage capturedMessage;

void eventHandler(byte index, const VLCB::VlcbMessage *msg)
{
  capturedIndex = index;
  capturedMessage = *msg;
}

void testEventHandlerOff()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventConsumerService->setEventHandler(eventHandler);

  // Add some long events
  configuration->writeEvent(0, 260, 0);
  configuration->updateEvHashEntry(0);

  configuration->writeEvent(1, 260, 1);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ACOF, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // No responses expected.
  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ACOF, capturedMessage.data[0]);
}

void testEventHandlerShortOn()
{
  test();
  capturedIndex = 0xFF;

  VLCB::Controller controller = createController();
  eventConsumerService->setEventHandler(eventHandler);

  // Add some short events
  configuration->writeEvent(0, 0, 1);
  configuration->updateEvHashEntry(0);
  configuration->writeEventEV(0, 1, 17);

  configuration->writeEvent(1, 0, 2);
  configuration->updateEvHashEntry(1);
  configuration->writeEventEV(1, 1, 42);

  VLCB::VlcbMessage msg = {5, {OPC_ASON, 0x01, 0x04, 0, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // No responses expected.
  assertEquals(0, mockTransportService->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ASON, capturedMessage.data[0]);
}

}

void testEventConsumerService()
{
  testServiceDiscovery();
  testServiceDiscoveryEventProdSvc();
  testEventHandlerOff();
  testEventHandlerShortOn();
}
