//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for CanService.
// * Service Discovery
// * CANID enumeration

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "CanService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "ArduinoMock.hpp"

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;

VLCB::Controller createController()
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  static std::unique_ptr<VLCB::CanService> canService;
  canService.reset(new VLCB::CanService);

  return ::createController({minimumNodeService.get(), canService.get()});
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
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
  assertEquals(SERVICE_ID_CAN, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryCanSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_CAN, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testCanidEnumerationOnUserAction()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setUninitialised(); // Clear all state as if module is factory fresh.
  minimumNodeService->setSetupMode();
  mockUserInterface->setRequestedAction(VLCB::UserInterface::ENUMERATION);

  // Check that CANID is unset on creation.
  assertEquals(0, controller.getModuleCANID());

  controller.process();
  mockUserInterface->setRequestedAction(VLCB::UserInterface::NONE);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(true, mockTransport->sent_messages[0].rtr);
  assertEquals(0, mockTransport->sent_messages[0].len);

  // Processing after enumeration timeout
  addMillis(101);
  mockTransport->sent_messages.clear();
  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());
}

void testCanidEnumerationOnENUM()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_ENUM, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(true, mockTransport->sent_messages[0].rtr);
  assertEquals(0, mockTransport->sent_messages[0].len);

  // Processing after enumeration timeout
  addMillis(101);
  mockTransport->sent_messages.clear();
  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NNACK, mockTransport->sent_messages[0].data[0]);

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());
}

void testRtrMessage()
{
  test();

  VLCB::Controller controller = createController();
  controller.getModuleConfig()->CANID = 3;

  VLCB::CANFrame msg = {0x11, false, true, 0, {}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(0, mockTransport->sent_messages[0].len);
  assertEquals(false, mockTransport->sent_messages[0].rtr);
  assertEquals(3, mockTransport->sent_messages[0].id);
}

void testFindFreeCanidOnPopulatedBus()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setUninitialised(); // Clear all state as if module is factory fresh.
  minimumNodeService->setSetupMode();
  mockUserInterface->setRequestedAction(VLCB::UserInterface::ENUMERATION);

  // Check that CANID is unset on creation.
  assertEquals(0, controller.getModuleCANID());

  controller.process();
  mockUserInterface->setRequestedAction(VLCB::UserInterface::NONE);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(true, mockTransport->sent_messages[0].rtr);
  assertEquals(0, mockTransport->sent_messages[0].len);
  mockTransport->sent_messages.clear();

  // Simulate other nodes
  for (byte remoteCanid = 1 ; remoteCanid <= 19 ; ++remoteCanid)
  {
    VLCB::CANFrame msg = {remoteCanid, false, false, 0, {}};
    mockTransport->setNextMessage(msg);
    controller.process();
    mockTransport->incoming_messages.clear();
  }
  assertEquals(0, mockTransport->sent_messages.size());
  
  // Processing after enumeration timeout
  addMillis(101);
  mockTransport->sent_messages.clear();
  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Expect first available CANID
  assertEquals(20, controller.getModuleCANID());
}

void testCANID()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 3, {OPC_CANID, 0x01, 0x04, 33}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  assertEquals(OPC_CANID, mockTransport->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_CAN, mockTransport->sent_messages[1].data[4]);
  assertEquals(GRSP_OK, mockTransport->sent_messages[1].data[5]);

  // Expect first available CANID
  assertEquals(33, controller.getModuleCANID());
}

}

void testCanService()
{
  testServiceDiscovery();
  testServiceDiscoveryCanSvc();
  testCanidEnumerationOnUserAction();
//  testCanidEnumerationOnPowerUp();
//  testCanidEnumerationOnSetUp();
//  testCanidEnumerationOnFirstSentMessage();
//  testCanidEnumerationOnConflict();
  testCanidEnumerationOnENUM(); // Deprecated
  testRtrMessage();
  testFindFreeCanidOnPopulatedBus();
  testCANID(); // Deprecated
}
