//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for ConsumeOwnEventsService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "ConsumeOwnEventsService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "EventConsumerService.h"
#include "EventProducerService.h"
#include "MockTransportService.h"

namespace
{
std::unique_ptr<VLCB::EventProducerService> eventProducerService;
std::unique_ptr<VLCB::EventConsumerService> eventConsumerService;

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransport.reset(new MockTransport);

  static std::unique_ptr<MockTransportService> mockTransportService;
  mockTransportService.reset(new MockTransportService(mockTransport.get()));

  static std::unique_ptr<VLCB::ConsumeOwnEventsService> consumeOwnEventsService;
  consumeOwnEventsService.reset(new VLCB::ConsumeOwnEventsService);

  eventConsumerService.reset(new VLCB::EventConsumerService);

  eventProducerService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), consumeOwnEventsService.get(),
                                                    eventProducerService.get(), eventConsumerService.get(), mockTransportService.get()});
  controller.begin();

  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(6, mockTransport->sent_messages.size());

  assertEquals(OPC_SD, mockTransport->sent_messages[0].data[0]);
  assertEquals(5, mockTransport->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransport->sent_messages[1].data[0]);
  assertEquals(1, mockTransport->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[2].data[0]);
  assertEquals(2, mockTransport->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_CONSUME_OWN_EVENTS, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_CONSUME_OWN_EVENTS, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testCoeFlag()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {1, {OPC_QNN}};
  mockTransport->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(MANU_VLCB, mockTransport->sent_messages[0].data[3]);
  assertEquals(MODULE_ID, mockTransport->sent_messages[0].data[4]);
  assertEquals(PF_CONSUMER | PF_PRODUCER | PF_NORMAL | PF_COE | PF_SD, mockTransport->sent_messages[0].data[5]);
}

byte capturedIndex = -1;
VLCB::VlcbMessage capturedMessage;

void eventHandler(byte index, const VLCB::VlcbMessage *msg)
{
  capturedIndex = index;
  capturedMessage = *msg;
}

void testConsumeOwnEvent()
{
  test();

  VLCB::Controller controller = createController();

  eventConsumerService->setEventHandler(eventHandler);

  // Add some long events
  byte eventData[] = {0x01, 0x04, 0x03, 0x02};
  configuration->writeEvent(0, eventData);
  configuration->writeEventEV(0, 1, 1);
  configuration->updateEvHashEntry(0);

  eventProducerService->sendEvent(true, 1);

  process(controller);
  
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(5, mockTransport->sent_messages[0].len);
  assertEquals(OPC_ACON, mockTransport->sent_messages[0].data[0]);

  process(controller);

  assertEquals(0, capturedIndex);
  assertEquals(5, capturedMessage.len);
  assertEquals(OPC_ACON, capturedMessage.data[0]);
}

}

void testConsumeOwnEventsService()
{
  testServiceDiscovery();
  testServiceDiscoveryEventProdSvc();
  testCoeFlag();
  testConsumeOwnEvent();
}
