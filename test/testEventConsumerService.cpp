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

namespace
{
std::unique_ptr<VLCB::EventConsumerService> eventConsumerService;

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  eventConsumerService.reset(new VLCB::EventConsumerService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), eventConsumerService.get()});
  controller.begin();
  
  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(3, mockTransport->sent_messages.size());

  assertEquals(OPC_SD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransport->sent_messages[1].data[0]);
  assertEquals(1, mockTransport->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[2].data[0]);
  assertEquals(2, mockTransport->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_CONSUMER, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_CONSUMER, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

byte capturedIndex;
VLCB::VlcbMessage capturedMessage;
bool capturedState;
byte capturedValue;

void eventHandler(byte index, VLCB::VlcbMessage *msg) 
{
  capturedIndex = index;
  capturedMessage = *msg;
}

void eventHandlerEx(byte index, VLCB::VlcbMessage *msg, bool evOn, byte evVal)
{
  capturedIndex = index;
  capturedMessage = *msg;
  capturedState = evOn;
  capturedValue = evVal;
}

void testEventHandlerOff()
{
  test();

  VLCB::Controller controller = createController();
  eventConsumerService->setEventHandler(eventHandler);
  
  // Add some long events
  byte eventData[] = {0x01, 0x04, 0x00, 0x00};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);
  
  eventData[3] = 0x01;
  configuration->writeEvent(1, eventData);
  configuration->updateEvHashEntry(1);

  VLCB::VlcbMessage msg = {5, {OPC_ACOF, 0x01, 0x04, 0, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // No responses expected.
  assertEquals(0, mockTransport->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ACOF, capturedMessage.data[0]);
}

void testEventHandlerExShortOn()
{
  test();

  VLCB::Controller controller = createController();
  eventConsumerService->setEventHandler(eventHandlerEx);

  // Add some short events
  byte eventData[] = {0x00, 0x00, 0x00, 0x01};
  configuration->writeEvent(0, eventData);
  configuration->updateEvHashEntry(0);
  configuration->writeEventEV(0, 1, 17);
  
  eventData[3] = 0x02;
  configuration->writeEvent(1, eventData);
  configuration->updateEvHashEntry(1);
  configuration->writeEventEV(1, 1, 42);

  VLCB::VlcbMessage msg = {5, {OPC_ASON, 0x01, 0x04, 0, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // No responses expected.
  assertEquals(0, mockTransport->sent_messages.size());

  assertEquals(1, capturedIndex);
  assertEquals(true, capturedState);
  assertEquals(42, capturedValue);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ASON, capturedMessage.data[0]);
}

}

void testEventConsumerService()
{
  testServiceDiscovery();
  testServiceDiscoveryEventProdSvc();
  testEventHandlerOff();
  testEventHandlerExShortOn();
}
