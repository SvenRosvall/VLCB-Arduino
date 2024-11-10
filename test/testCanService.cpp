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
#include "MockCanTransport.h"

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;

// Use MockCanTransport to test CanTransport class.
std::unique_ptr<MockCanTransport> mockCanTransport;

VLCB::Controller createController()
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockCanTransport.reset(new MockCanTransport);

  static std::unique_ptr<VLCB::CanService> canService;
  canService.reset(new VLCB::CanService(mockCanTransport.get()));

  VLCB::Controller controller = ::createController({minimumNodeService.get(), canService.get()});
  controller.begin();

  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(3, mockCanTransport->sent_frames.size());

  assertEquals(OPC_SD, mockCanTransport->sent_frames[0].data[0]);
  assertEquals(2, mockCanTransport->sent_frames[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockCanTransport->sent_frames[1].data[0]);
  assertEquals(1, mockCanTransport->sent_frames[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockCanTransport->sent_frames[1].data[4]); // service ID
  assertEquals(1, mockCanTransport->sent_frames[1].data[5]); // version

  assertEquals(OPC_SD, mockCanTransport->sent_frames[2].data[0]);
  assertEquals(2, mockCanTransport->sent_frames[2].data[3]); // index
  assertEquals(SERVICE_ID_CAN, mockCanTransport->sent_frames[2].data[4]); // service ID
  assertEquals(2, mockCanTransport->sent_frames[2].data[5]); // version
}

void testServiceDiscoveryCanSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(OPC_ESD, mockCanTransport->sent_frames[0].data[0]);
  assertEquals(2, mockCanTransport->sent_frames[0].data[3]); // index
  assertEquals(SERVICE_ID_CAN, mockCanTransport->sent_frames[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testCanidEnumerationOnUserAction()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setUninitialised(); // Clear all state as if module is factory fresh.
  minimumNodeService->setSetupMode();

  // Check that CANID is unset on creation.
  assertEquals(0, controller.getModuleCANID());

  controller.putAction({VLCB::ACT_START_CAN_ENUMERATION});

  process(controller);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(true, mockCanTransport->sent_frames[0].rtr);
  assertEquals(0, mockCanTransport->sent_frames[0].len);

  // Processing after enumeration timeout
  addMillis(101);
  mockCanTransport->sent_frames.clear();
  process(controller);

  assertEquals(0, mockCanTransport->sent_frames.size());

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());
}

void testCanidEnumerationOnSetUp()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setUninitialised(); // Clear all state as if module is factory fresh.

  // Check that CANID is unset on creation.
  assertEquals(0, controller.getModuleCANID());

  controller.putAction({VLCB::ACT_CHANGE_MODE});

  process(controller);

  // Expect message to make other modules report their CANID's
  assertEquals(2, mockCanTransport->sent_frames.size());

  assertEquals(true, mockCanTransport->sent_frames[0].rtr);
  assertEquals(0, mockCanTransport->sent_frames[0].len);

  assertEquals(OPC_RQNN, mockCanTransport->sent_frames[1].data[0]);
  assertEquals(0, mockCanTransport->sent_frames[1].data[1]);
  assertEquals(0, mockCanTransport->sent_frames[1].data[2]);

  // Processing after enumeration timeout
  addMillis(101);
  mockCanTransport->sent_frames.clear();
  process(controller);

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());

  // Expect no further messages
  assertEquals(0, mockCanTransport->sent_frames.size());

  // Note: RQNN shouldn't really be sent until a CANID has been set.
  // But this works so going to leave as is.
}

void testCanidEnumerationOnENUM()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_ENUM, 0x01, 0x04, 2}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(true, mockCanTransport->sent_frames[0].rtr);
  assertEquals(0, mockCanTransport->sent_frames[0].len);

  // Processing after enumeration timeout
  addMillis(101);
  mockCanTransport->sent_frames.clear();
  process(controller);

  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(OPC_NNACK, mockCanTransport->sent_frames[0].data[0]);

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());
}

void testCanidEnumerationOnConflict()
{
  test();

  VLCB::Controller controller = createController();
  controller.getModuleConfig()->setCANID(3);

  VLCB::CANFrame msg = {3, false, false, 1, {OPC_RQNP}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(true, mockCanTransport->sent_frames[0].rtr);
  assertEquals(0, mockCanTransport->sent_frames[0].len);

  // Processing after enumeration timeout
  addMillis(101);
  mockCanTransport->sent_frames.clear();
  process(controller);

  assertEquals(0, mockCanTransport->sent_frames.size());

  // Expect first available CANID
  assertEquals(1, controller.getModuleCANID());
}

void testRtrMessage()
{
  test();

  VLCB::Controller controller = createController();
  controller.getModuleConfig()->CANID = 3;

  VLCB::CANFrame msg = {0x11, false, true, 0, {}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(0, mockCanTransport->sent_frames[0].len);
  assertEquals(false, mockCanTransport->sent_frames[0].rtr);
  assertEquals(3, mockCanTransport->sent_frames[0].id & 0x7F);
}

void testFindFreeCanidOnPopulatedBus()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setUninitialised(); // Clear all state as if module is factory fresh.
  minimumNodeService->setSetupMode();

  // Check that CANID is unset on creation.
  assertEquals(0, controller.getModuleCANID());

  controller.putAction({VLCB::ACT_START_CAN_ENUMERATION});

  process(controller);

  // Expect message to make other modules report their CANID's
  assertEquals(1, mockCanTransport->sent_frames.size());
  assertEquals(true, mockCanTransport->sent_frames[0].rtr);
  assertEquals(0, mockCanTransport->sent_frames[0].len);
  mockCanTransport->sent_frames.clear();

  // Simulate other nodes
  for (byte remoteCanid = 1 ; remoteCanid <= 19 ; ++remoteCanid)
  {
    VLCB::CANFrame msg = {remoteCanid, false, false, 0, {}};
    mockCanTransport->setNextMessage(msg);
    controller.process();
    mockCanTransport->incoming_frames.clear();
  }
  assertEquals(0, mockCanTransport->sent_frames.size());

  // Processing after enumeration timeout
  addMillis(101);
  mockCanTransport->sent_frames.clear();
  process(controller);

  assertEquals(0, mockCanTransport->sent_frames.size());

  // Expect first available CANID
  assertEquals(20, controller.getModuleCANID());
}

void testCANID()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, false, false, 4, {OPC_CANID, 0x01, 0x04, 33}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockCanTransport->sent_frames.size());
  assertEquals(OPC_WRACK, mockCanTransport->sent_frames[0].data[0]);
  assertEquals(OPC_GRSP, mockCanTransport->sent_frames[1].data[0]);
  assertEquals(OPC_CANID, mockCanTransport->sent_frames[1].data[3]);
  assertEquals(SERVICE_ID_CAN, mockCanTransport->sent_frames[1].data[4]);
  assertEquals(GRSP_OK, mockCanTransport->sent_frames[1].data[5]);

  // Expect first available CANID
  assertEquals(33, controller.getModuleCANID());
}

void testRequestAllDiagnosticsCanService()
{
  test();

  VLCB::Controller controller = createController();

  // Request all diagnostics for MockCanService as it does not implement any diagnostics.
  const byte serviceIndex = 2;
  VLCB::CANFrame msg = {0x11, false, false, 5, {OPC_RDGN, 0x01, 0x04, serviceIndex, 0}};
  mockCanTransport->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(19, mockCanTransport->sent_frames.size());

  int messageIndex = 0;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(18, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(1, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(2, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(3, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(4, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(5, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(6, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(42, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(7, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(8, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(9, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(17, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(10, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(11, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(12, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(13, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(14, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(15, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(16, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(17, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockCanTransport->sent_frames[messageIndex].data[0]);
  assertEquals(serviceIndex, mockCanTransport->sent_frames[messageIndex].data[3]);
  assertEquals(18, mockCanTransport->sent_frames[messageIndex].data[4]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[5]);
  assertEquals(0, mockCanTransport->sent_frames[messageIndex].data[6]);
}

}

void testCanService()
{
  testServiceDiscovery();
  testServiceDiscoveryCanSvc();
  testCanidEnumerationOnUserAction();
  testCanidEnumerationOnSetUp();
  testCanidEnumerationOnConflict();
  testCanidEnumerationOnENUM(); // Deprecated
  testRtrMessage();
  testFindFreeCanidOnPopulatedBus();
  testCANID(); // Deprecated
  testRequestAllDiagnosticsCanService();
}
