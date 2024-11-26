//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for MinimumNodeService.
// MNS implements these OP-codes.
// * RQNN - (Initiated by module from user action.)
// * NNREL - (Initiated by module from user action.)
// * SNN - Done
// * QNN - Done
// * RQNP - Done
// * RQMN - Done
// * RQNPN - Done
// * RDGN - Done
// * RQSD - Done
// * MODE - Done in MNS.
// * SQU - ????
// * NNRST - Done
// * NNRSM - Done

#include <memory>
#include "TestTools.hpp"
#include "ArduinoMock.hpp"
#include "Controller.h"
#include "MinimumNodeServiceWithDiagnostics.h"
#include "LongMessageService.h"
#include "EventConsumerService.h"
#include "EventProducerService.h"
#include "VlcbCommon.h"
#include "MockTransportService.h"

namespace
{
std::unique_ptr<VLCB::MinimumNodeServiceWithDiagnostics> minimumNodeService;
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController(VlcbModeParams startupMode = MODE_NORMAL)
{
  minimumNodeService.reset(new VLCB::MinimumNodeServiceWithDiagnostics);
  
  mockUserInterface.reset(new MockUserInterface);

  mockTransportService.reset(new MockTransportService);

  static std::unique_ptr<VLCB::LongMessageService> longMessageService;
  longMessageService.reset(new VLCB::LongMessageService);

  static std::unique_ptr<VLCB::EventConsumerService> ecService;
  ecService.reset(new VLCB::EventConsumerService);

  static std::unique_ptr<VLCB::EventProducerService> epService;
  epService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller = ::createController(startupMode, {minimumNodeService.get(), mockUserInterface.get(),
       ecService.get(), epService.get(), longMessageService.get(), mockTransportService.get()});
  controller.begin();
  minimumNodeService->setHeartBeat(false);

  return controller;
}

void testUninitializedRequestNodeNumber()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);

  // User requests to enter Setup mode.
  controller.putAction({VLCB::ACT_CHANGE_MODE});

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_RQNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0, mockTransportService->sent_messages[0].data[2]);

  assertEquals(MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0, configuration->nodeNum);
}

void testNormalRequestNodeNumber()
{
  test();

  VLCB::Controller controller = createController();

  // User requests to enter Setup mode.
  controller.putAction({VLCB::ACT_RENEGOTIATE});

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(OPC_RQNN, mockTransportService->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[1].data[2]);

  assertEquals(MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_NORMAL, configuration->currentMode);
  assertEquals(0x104, configuration->nodeNum);
}

void testNormalChangeModeToUninitialized()
{
  // Send an NNACK if no SNN received within 30 seconds from RENEGOTIATE action.

  test();

  VLCB::Controller controller = createController();

  // User requests to change mode.
  controller.putAction({VLCB::ACT_CHANGE_MODE});

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);

  assertEquals(MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0x0, configuration->nodeNum);
}

void testNormalChangeModeToSetup()
{
  test();

  VLCB::Controller controller = createController();

  // User requests to enter Setup mode.
  controller.putAction({VLCB::ACT_RENEGOTIATE});

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(OPC_RQNN, mockTransportService->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[1].data[2]);

  assertEquals(MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_NORMAL, configuration->currentMode);

  // Module will hold on to node number in case setup times out.
  assertEquals(0x104, configuration->nodeNum);
}

void testSetupFromUninitializedReceiveRequestNodeNumberElsewhere()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQNN, 0x02, 0x07}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Expect module to not respond, but to change to non-setup mode.

  assertEquals(0, mockTransportService->sent_messages.size());

  // TODO: Need a better way of determining the instantMode.
  assertEquals(MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
}

void testSetupFromNormalReceiveRequestNodeNumberElsewhere()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQNN, 0x02, 0x07}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Expect module to not respond, but to change to non-setup mode.

  assertEquals(0, mockTransportService->sent_messages.size());

  // TODO: Need a better way of determining the instantMode.
  assertEquals(MODE_NORMAL, mockUserInterface->getIndicatedMode());
}

void testSetupFromUninitializedAbort()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);
  minimumNodeService->setSetupMode();

  controller.putAction({VLCB::ACT_RENEGOTIATE});

  process(controller);

  // Expect module to not respond, but to change to non-setup mode.

  assertEquals(0, mockTransportService->sent_messages.size());

  // TODO: Need a better way of determining the instantMode.
  assertEquals(MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
}

void testSetupFromNormalAbort()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  controller.putAction({VLCB::ACT_RENEGOTIATE});

  process(controller);

  // Expect module to not respond, but to change to non-setup mode.
  // TODO Question: Shouldn't this reclaim NN with NNACK?
  assertEquals(0, mockTransportService->sent_messages.size());

  // TODO: Need a better way of determining the instantMode.
  assertEquals(MODE_NORMAL, mockUserInterface->getIndicatedMode());
}

void testSetNodeNumber()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_SNN, 0x02, 0x07}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NNACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x02, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[2]);
}

void testSetNodeNumberNormal()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_SNN, 0x02, 0x07}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testSetNodeNumberShort()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {2, {OPC_SNN, 0x02}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(6, mockTransportService->sent_messages[0].len);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_SNN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testQueryNodeNumber()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {1, {OPC_QNN}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(MANU_MERG_VLCB, mockTransportService->sent_messages[0].data[3]);
  assertEquals(MODULE_ID, mockTransportService->sent_messages[0].data[4]);
  assertEquals(PF_CONSUMER | PF_PRODUCER | PF_NORMAL | PF_VLCB, mockTransportService->sent_messages[0].data[5]);
}

void testReadNodeParametersNormalMode()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {1, {OPC_RQNP}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testReadNodeParametersSetupMode()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {1, {OPC_RQNP}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PARAMS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(MANU_MERG_VLCB, mockTransportService->sent_messages[0].data[1]);
  assertEquals(1, mockTransportService->sent_messages[0].data[2]); // Minor version
  assertEquals(MODULE_ID, mockTransportService->sent_messages[0].data[3]);
  assertEquals(1, mockTransportService->sent_messages[0].data[7]); // Major version
}

void testReadNodeParameterCount()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQNPN, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(21, mockTransportService->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]); // Parameter index
  assertEquals(20, mockTransportService->sent_messages[0].data[4]); // Number of parameters

  // Manufacturer
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[1].data[0]);
  assertEquals(1, mockTransportService->sent_messages[1].data[3]);
  assertEquals(MANU_MERG_VLCB, mockTransportService->sent_messages[1].data[4]);

  // Flags
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[8].data[0]);
  assertEquals(8, mockTransportService->sent_messages[8].data[3]);
  assertEquals(PF_COMBI | PF_NORMAL | PF_VLCB, mockTransportService->sent_messages[8].data[4]);
}

void testReadNodeParameterModuleId()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQNPN, 0x01, 0x04, 3}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(3, mockTransportService->sent_messages[0].data[3]); // Number of services
  assertEquals(MODULE_ID, mockTransportService->sent_messages[0].data[4]); // Number of services
}

void testReadNodeParameterInvalidIndex()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQNPN, 0x01, 0x04, 33}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_PARAM_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_RQNPN, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_PARAM_IDX, mockTransportService->sent_messages[1].data[5]);
}

void testReadNodeParameterShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQNPN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RQNPN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testModuleNameSetup()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQMN}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NAME, mockTransportService->sent_messages[0].data[0]);
  assertEquals(moduleName[0], mockTransportService->sent_messages[0].data[1]);
  assertEquals(moduleName[1], mockTransportService->sent_messages[0].data[2]);
  assertEquals(moduleName[2], mockTransportService->sent_messages[0].data[3]);
  assertEquals(moduleName[3], mockTransportService->sent_messages[0].data[4]);
  assertEquals(moduleName[4], mockTransportService->sent_messages[0].data[5]);
  assertEquals(moduleName[5], mockTransportService->sent_messages[0].data[6]);
  assertEquals(moduleName[6], mockTransportService->sent_messages[0].data[7]);
}

void testModuleNameLearn()
{
  test();

  VLCB::Controller controller = createController();
  controller.setParamFlag(PF_LRN, true); // Should really use the event teaching service here.

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQMN}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NAME, mockTransportService->sent_messages[0].data[0]);
}

void testModuleNameNormal()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQMN}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testHeartBeat()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setHeartBeat(true);

  // Not expecting a heartbeat before 5 seconds.
  addMillis(4500);
  process(controller);
  assertEquals(0, mockTransportService->sent_messages.size());

  // But expecting a heartbeat after 5 seconds.
  addMillis(1000);
  process(controller);
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_HEARTB, mockTransportService->sent_messages[0].data[0]);
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  size_t serviceCount = controller.getServices().size();
  size_t expectedResponseServices = serviceCount - 1; // Omit the user interface.
  // Verify sent messages.
  assertEquals(expectedResponseServices + 1, mockTransportService->sent_messages.size());

  assertEquals(OPC_SD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(expectedResponseServices, mockTransportService->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransportService->sent_messages[1].data[0]);
  assertEquals(1, mockTransportService->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransportService->sent_messages[2].data[0]);
  assertEquals(3, mockTransportService->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_CONSUMER, mockTransportService->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[2].data[5]); // version

  assertEquals(OPC_SD, mockTransportService->sent_messages[3].data[0]);
  assertEquals(4, mockTransportService->sent_messages[3].data[3]); // index
  assertEquals(SERVICE_ID_PRODUCER, mockTransportService->sent_messages[3].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[3].data[5]); // version

  assertEquals(OPC_SD, mockTransportService->sent_messages[4].data[0]);
  assertEquals(5, mockTransportService->sent_messages[4].data[3]); // index
  assertEquals(SERVICE_ID_STREAMING, mockTransportService->sent_messages[4].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[4].data[5]); // version
}

void testServiceDiscoveryLongMessageSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQSD, 0x01, 0x04, 5}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ESD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(5, mockTransportService->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_STREAMING, mockTransportService->sent_messages[0].data[4]);
  // Not testing service data bytes.
}

void testServiceDiscoveryIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQSD, 0x01, 0x04, 7}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RQSD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_SERVICE, mockTransportService->sent_messages[0].data[5]);
}

void testServiceDiscoveryIndexForUI()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RQSD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_SERVICE, mockTransportService->sent_messages[0].data[5]);
}

void testServiceDiscoveryShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_RQSD, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RQSD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testRequestDiagnosticsIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, 7, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RDGN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_SERVICE, mockTransportService->sent_messages[0].data[5]);
}

void testRequestDiagnosticsIndexForUI()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, 2, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RDGN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_SERVICE, mockTransportService->sent_messages[0].data[5]);
}

void testRequestDiagnosticsShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_RDGN, 0x01, 0x04, 1}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RDGN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testRequestAllDiagnosticsIndexForMockService()
{
  test();

  VLCB::Controller controller = createController();

  // Request all diagnostics for MockCanService as it does not implement any diagnostics.
  const byte mockServiceIndex = 6;
  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, mockServiceIndex, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_DGN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(mockServiceIndex, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5]);  
  assertEquals(0, mockTransportService->sent_messages[0].data[6]);  
}

void testRequestInvalidDiagnosticsIndexForMockService()
{
  test();

  VLCB::Controller controller = createController();

  // Request all diagnostics for MockCanService as it does not implement any diagnostics.
  const byte mockServiceIndex = 6;
  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, mockServiceIndex, 7}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_RDGN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_DIAGNOSTIC, mockTransportService->sent_messages[0].data[5]);
}

void testRequestAllDiagnosticsAllServices()
{
  test();

  const byte serviceIndexMns = 1, serviceIndexEC = 3, serviceIndexEP = 4, serviceIndexLMS = 5, serviceIndexTransport = 6;
  VLCB::Controller controller = createController();

  // Request all diagnostics for MockCanService as it does not implement any diagnostics.
  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, 0, 0}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(11, mockTransportService->sent_messages.size());

  int messageIndex = 0;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(6, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(1, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(2, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(3, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(4, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(5, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexMns, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(6, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexEC, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexEP, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexLMS, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);

  ++messageIndex;
  assertEquals(OPC_DGN, mockTransportService->sent_messages[messageIndex].data[0]);
  assertEquals(serviceIndexTransport, mockTransportService->sent_messages[messageIndex].data[3]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[4]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[5]);
  assertEquals(0, mockTransportService->sent_messages[messageIndex].data[6]);
}

void testRequestMnsDiagnosticsUptime()
{
  test();

  addMillis(123456789);

  VLCB::Controller controller = createController();

  // Request upper word of uptime
  VLCB::VlcbMessage msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, SERVICE_ID_MNS, 2}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_DGN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[3]);
  assertEquals(2, mockTransportService->sent_messages[0].data[4]);
  unsigned long uptime = (mockTransportService->sent_messages[0].data[5] << 24)
          + (mockTransportService->sent_messages[0].data[6] << 16);

  // Request lower word of uptime
  mockTransportService->sent_messages.clear();
  msg_rqsd = {5, {OPC_RDGN, 0x01, 0x04, SERVICE_ID_MNS, 3}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_DGN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[3]);
  assertEquals(3, mockTransportService->sent_messages[0].data[4]);
  uptime += (mockTransportService->sent_messages[0].data[5] << 8)
            + (mockTransportService->sent_messages[0].data[6]);

  assertEquals(123456, uptime);
}

void testRequestMnsDiagnosticsNodeNumberChanges()
{
  test();

  VLCB::Controller controller = createController();

  // Change node number
  minimumNodeService->setSetupMode();
  VLCB::VlcbMessage msg_rqsd = {3, {OPC_SNN, 0x02, 0x07}};
  mockTransportService->setNextMessage(msg_rqsd);
  process(controller);
  mockTransportService->sent_messages.clear();

  // Request upper word of uptime
  msg_rqsd = {5, {OPC_RDGN, 0x02, 0x07, SERVICE_ID_MNS, 5}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_DGN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[3]);
  assertEquals(5, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5]);
  assertEquals(1, mockTransportService->sent_messages[0].data[6]);
}

void testModeSetupFromUninitializedToNormal()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_NORMAL}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_MODE, mockTransportService->sent_messages[0].data[5]);

  assertEquals(MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0, configuration->nodeNum);
}

void testModeSetupFromNormalToNormal()
{
  test();

  VLCB::Controller controller = createController(MODE_NORMAL);
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0x01, 0x04, MODE_NORMAL}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_MODE, mockTransportService->sent_messages[0].data[5]);

  assertEquals(MODE_NORMAL, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_NORMAL, configuration->currentMode);
  assertEquals(0x104, configuration->nodeNum);
}

void testModeSetupToUnininitialized()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_UNINITIALISED}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_OK, mockTransportService->sent_messages[0].data[5]);

  assertEquals(MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0, configuration->nodeNum);
}

void testModeNormalToSetup()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0x01, 0x04, MODE_SETUP}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(3, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_OK, mockTransportService->sent_messages[0].data[5]);
  // NN should be 0.

  assertEquals(OPC_NNREL, mockTransportService->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[1].data[2]);

  assertEquals(OPC_RQNN, mockTransportService->sent_messages[2].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[2].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[2].data[2]);

  assertEquals(MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(MODE_NORMAL, configuration->currentMode);
  assertEquals(0x104, configuration->nodeNum);
}

void testModeUninitializedToSetup()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_SETUP}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testModeUninitializedToNormal()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_NORMAL}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testModeUninitializedToUninitialized()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_UNINITIALISED}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
}

void testModeSetupToSetup()
{
  test();

  VLCB::Controller controller = createController(MODE_UNINITIALISED);
  minimumNodeService->setSetupMode();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0, 0, MODE_SETUP}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_MODE, mockTransportService->sent_messages[0].data[5]);
}

void testModeNormalToNormal()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {4, {OPC_MODE, 0x01, 0x04, MODE_NORMAL}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(GRSP_OK, mockTransportService->sent_messages[0].data[5]);
}

void testModeShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg_rqsd = {3, {OPC_MODE, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg_rqsd);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

}

void testMinimumNodeService()
{
  testUninitializedRequestNodeNumber();
  testNormalRequestNodeNumber();
  testNormalChangeModeToUninitialized();
  testNormalChangeModeToSetup();
  testSetupFromUninitializedReceiveRequestNodeNumberElsewhere();
  testSetupFromNormalReceiveRequestNodeNumberElsewhere();
  testSetupFromUninitializedAbort();
  testSetupFromNormalAbort();
  testSetNodeNumber();
  testSetNodeNumberNormal();
  testSetNodeNumberShort();
  testQueryNodeNumber();
  testReadNodeParametersNormalMode();
  testReadNodeParametersSetupMode();
  testReadNodeParameterCount();
  testReadNodeParameterModuleId();
  testReadNodeParameterInvalidIndex();
  testReadNodeParameterShortMessage();
  testModuleNameSetup();
  testModuleNameLearn();
  testModuleNameNormal();
  testHeartBeat();
  testServiceDiscovery();
  testServiceDiscoveryLongMessageSvc();
  testServiceDiscoveryIndexOutOfBand();
  testServiceDiscoveryIndexForUI();
  testServiceDiscoveryShortMessage();
  testRequestDiagnosticsIndexOutOfBand();
  testRequestDiagnosticsIndexForUI();
  testRequestDiagnosticsShortMessage();
  testRequestAllDiagnosticsIndexForMockService();
  testRequestInvalidDiagnosticsIndexForMockService();
  testRequestAllDiagnosticsAllServices();
  testRequestMnsDiagnosticsUptime();
  testRequestMnsDiagnosticsNodeNumberChanges();
  testModeSetupFromUninitializedToNormal();
  testModeSetupFromNormalToNormal();
  testModeSetupToUnininitialized();
  testModeNormalToSetup();
  testModeUninitializedToSetup();
  testModeUninitializedToNormal();
  testModeUninitializedToUninitialized();
  testModeSetupToSetup();
  testModeNormalToNormal();
  testModeShortMessage();
}
