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

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
std::unique_ptr<VLCB::EventProducerService> eventProducerService;

VLCB::Controller createController()
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  eventProducerService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), eventProducerService.get()});
  
  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

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
  assertEquals(SERVICE_ID_PRODUCER, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_PRODUCER, mockTransport->sent_messages[0].data[4]); // service ID
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

  eventProducerService->sendEvent(true, 0);

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(5, mockTransport->sent_messages[0].len);
  assertEquals(OPC_ACON, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[4]);
}

void testSend1Off()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  byte nnen[] = { 0x01, 0x04, 0x00, 0x01};
  controller.getModuleConfig()->writeEvent(0, nnen);

  eventProducerService->sendEvent(false, 0, 42);

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ACOF1, mockTransport->sent_messages[0].data[0]);
  assertEquals(6, mockTransport->sent_messages[0].len);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[4]);
  assertEquals(42, mockTransport->sent_messages[0].data[5]);
}

void testSendShort2On()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  byte nnen[] = { 0x00, 0x00, 0x00, 0x05};
  controller.getModuleConfig()->writeEvent(0, nnen);

  eventProducerService->sendEvent(true, 0, 42, 17);

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ASON2, mockTransport->sent_messages[0].data[0]);
  assertEquals(7, mockTransport->sent_messages[0].len);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[4]);
  assertEquals(42, mockTransport->sent_messages[0].data[5]);
  assertEquals(17, mockTransport->sent_messages[0].data[6]);
}

void testSendShort3Off()
{
  test();

  VLCB::Controller controller = createController();
  controller.begin();

  // Initialize a produced event
  byte nnen[] = { 0x00, 0x00, 0x00, 0x05};
  controller.getModuleConfig()->writeEvent(0, nnen);

  eventProducerService->sendEvent(false, 0, 42, 17, 234);

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ASOF3, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].len);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x00, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[4]);
  assertEquals(42, mockTransport->sent_messages[0].data[5]);
  assertEquals(17, mockTransport->sent_messages[0].data[6]);
  assertEquals(234, mockTransport->sent_messages[0].data[7]);
}

void testSetProducedDefaultEventsOnNewBoard()
{
  test();

  VLCB::Controller controller = createController();

  // Set module into Uninitialized mode to get unitialized memory.
  minimumNodeService->setUninitialised();

  controller.begin();
  
  minimumNodeService->setNormal();
  
  // Should have no events configured at start
  assertEquals(0, controller.getModuleConfig()->numEvents());

  controller.process();

  assertEquals(1, controller.getModuleConfig()->numEvents());
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
}
