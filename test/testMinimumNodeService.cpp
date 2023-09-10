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
// * RDGN - No support in MNS yet
// * RQSD - Done - Tests done
// * MODE - No support in MNS yet
// * SQU - ????
// * NNRST - Done
// * NNRSM - Done

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MockUserInterface.h"
#include "MinimumNodeService.h"
#include "MockTransport.h"
#include "MockStorage.h"
#include "vlcbdefs.hpp"
#include "Parameters.h"
#include "LongMessageService.h"
#include "ArduinoMock.hpp"
#include "EventConsumerService.h"
#include "EventProducerService.h"

namespace
{
const int MODULE_ID = 253;
unsigned char moduleName[] = {'t', 'e', 's', 't', 'i', 'n', 'g', '\0'};

static std::unique_ptr<MockUserInterface> mockUserInterface;
static std::unique_ptr<MockTransport> mockTransport;
static std::unique_ptr<VLCB::Configuration> configuration;
static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;

VLCB::Controller createController()
{
  // Use pointers to objects to create the controller with.
  // Use unique_ptr so that next invocation deletes the previous objects.
  mockTransport.reset(new MockTransport);

  mockUserInterface.reset(new MockUserInterface);

  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);

  configuration.reset(new VLCB::Configuration(mockStorage.get()));

  minimumNodeService.reset(new VLCB::MinimumNodeService);
  minimumNodeService->setHeartBeat(false);

  static std::unique_ptr<VLCB::LongMessageService> longMessageService;
  longMessageService.reset(new VLCB::LongMessageService);

  static std::unique_ptr<VLCB::EventConsumerService> ecService;
  ecService.reset(new VLCB::EventConsumerService);

  static std::unique_ptr<VLCB::EventProducerService> epService;
  epService.reset(new VLCB::EventProducerService);

  VLCB::Controller controller(mockUserInterface.get(), configuration.get(), mockTransport.get(),
     {minimumNodeService.get(), ecService.get(), epService.get(), longMessageService.get()});

  configuration->EE_NVS_START = 0;
  configuration->EE_NUM_NVS = 4;
  configuration->EE_EVENTS_START = 20;
  configuration->EE_MAX_EVENTS = 20;
  configuration->EE_PRODUCED_EVENTS = 1;
  configuration->EE_NUM_EVS = 2;
  configuration->setModuleMode(VLCB::MODE_NORMAL);
  configuration->setNodeNum(0x0104);
  static std::unique_ptr<VLCB::Parameters> params;
  params.reset(new VLCB::Parameters(*configuration));
  params->setVersion(1, 1, 'a');
  params->setModuleId(MODULE_ID);

  // assign to Controller
  controller.setParams(params->getParams());
  controller.setName(moduleName);
  controller.begin();
  return controller;
}

void testUninitializedRequestNodeNumber()
{
  test();

  VLCB::Controller controller = createController();

  // Set module into Uninitialized mode:
  configuration->currentMode = VLCB::MODE_UNINITIALISED;
  configuration->setNodeNum(0);
  
  // User requests to enter Setup mode.
  mockUserInterface->setRequestedAction(VLCB::UserInterface::CHANGE_MODE);
  
  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_RQNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[1]);
  assertEquals(0, mockTransport->sent_messages[0].data[2]);

  assertEquals(VLCB::MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(VLCB::MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0, configuration->nodeNum);
}

void testUninitializedRequestNodeNumberMissingSNN()
{
  // Send an NNACK if no SNN received within 30 seconds from RENEGOTIATE action.

  test();

  VLCB::Controller controller = createController();

  // Set module into Uninitialized mode:
  configuration->currentMode = VLCB::MODE_UNINITIALISED;
  configuration->setNodeNum(0);

  // User requests to enter Normal mode.
  mockUserInterface->setRequestedAction(VLCB::UserInterface::CHANGE_MODE);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_RQNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[1]);
  assertEquals(0, mockTransport->sent_messages[0].data[2]);

  assertEquals(VLCB::MODE_SETUP, mockUserInterface->getIndicatedMode());

  mockTransport->sent_messages.clear();
  addMillis(31 * 1000);
  mockUserInterface->setRequestedAction(VLCB::UserInterface::NONE);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  //assertEquals(OPC_NNACK, mockTransport->sent_messages[0].data[0]);

  assertEquals(VLCB::MODE_UNINITIALISED, mockUserInterface->getIndicatedMode());
  assertEquals(VLCB::MODE_UNINITIALISED, configuration->currentMode);
  assertEquals(0, configuration->nodeNum);
}

void testNormalRequestNodeNumber()
{
  test();

  VLCB::Controller controller = createController();

  // User requests to enter Setup mode.
  mockUserInterface->setRequestedAction(VLCB::UserInterface::CHANGE_MODE);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(OPC_RQNN, mockTransport->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[1].data[2]);

  assertEquals(VLCB::MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(VLCB::MODE_NORMAL, configuration->currentMode);
  assertEquals(0x104, configuration->nodeNum);
}

void testNormalRequestNodeNumberMissingSNN()
{
  // Send an NNACK if no SNN received within 30 seconds from RENEGOTIATE action.

  test();

  VLCB::Controller controller = createController();

  // User requests to change mode.
  mockUserInterface->setRequestedAction(VLCB::UserInterface::CHANGE_MODE);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);
  assertEquals(OPC_RQNN, mockTransport->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[1].data[2]);

  assertEquals(VLCB::MODE_SETUP, mockUserInterface->getIndicatedMode());

  mockTransport->sent_messages.clear();
  addMillis(31 * 1000);
  mockUserInterface->setRequestedAction(VLCB::UserInterface::NONE);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NNACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);

  assertEquals(VLCB::MODE_NORMAL, mockUserInterface->getIndicatedMode());
  assertEquals(VLCB::MODE_NORMAL, configuration->currentMode);
  assertEquals(0x104, configuration->nodeNum);
}

void testReleaseNodeNumberByUI()
{
  test();

  VLCB::Controller controller = createController();

  mockUserInterface->setRequestedAction(VLCB::UserInterface::CHANGE_MODE);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_NNREL, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[0].data[2]);

  assertEquals(OPC_RQNN, mockTransport->sent_messages[1].data[0]);
  assertEquals(0x01, mockTransport->sent_messages[1].data[1]);
  assertEquals(0x04, mockTransport->sent_messages[1].data[2]);

  assertEquals(VLCB::MODE_SETUP, mockUserInterface->getIndicatedMode());
  assertEquals(VLCB::MODE_NORMAL, configuration->currentMode);

  // Module will hold on to node number in case setup times out.
  assertEquals(0x104, configuration->nodeNum);
}

void testRequestNodeNumberElsewhere()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_RQNN, 0x02, 0x07}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Expect module to not respond, but to change to non-setup mode.

  assertEquals(0, mockTransport->sent_messages.size());

  // TODO: Need a better way of determining the instantMode.
  assertEquals(VLCB::MODE_NORMAL, mockUserInterface->getIndicatedMode());
}

void testSetNodeNumber()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_SNN, 0x02, 0x07}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NNACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x02, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[2]);
}

void testSetNodeNumberNormal()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_SNN, 0x02, 0x07}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
}

void testSetNodeNumberShort()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 2, {OPC_SNN, 0x02}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_SNN, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

void testReadNodeParametersNormalMode()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 1, {OPC_RQNP}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
}

void testReadNodeParametersSetupMode()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 1, {OPC_RQNP}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAMS, mockTransport->sent_messages[0].data[0]);
  assertEquals(MANU_MERG, mockTransport->sent_messages[0].data[1]);
  assertEquals(1, mockTransport->sent_messages[0].data[2]); // Minor version
  assertEquals(MODULE_ID, mockTransport->sent_messages[0].data[3]);
  assertEquals(1, mockTransport->sent_messages[0].data[7]); // Major version
}

void testReadNodeParameterCount()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQNPN, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(21, mockTransport->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[3]); // Parameter index
  assertEquals(20, mockTransport->sent_messages[0].data[4]); // Number of parameters

  // Manufacturer
  assertEquals(OPC_PARAN, mockTransport->sent_messages[1].data[0]);
  assertEquals(1, mockTransport->sent_messages[1].data[3]);
  assertEquals(MANU_MERG, mockTransport->sent_messages[1].data[4]);
  
  // Flags
  assertEquals(OPC_PARAN, mockTransport->sent_messages[8].data[0]);
  assertEquals(8, mockTransport->sent_messages[8].data[3]);
  assertEquals(PF_COMBI | PF_NORMAL, mockTransport->sent_messages[8].data[4]);
}

void testReadNodeParameterModuleId()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQNPN, 0x01, 0x04, 3}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(3, mockTransport->sent_messages[0].data[3]); // Number of services
  assertEquals(MODULE_ID, mockTransport->sent_messages[0].data[4]); // Number of services
}

void testReadNodeParameterInvalidIndex()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQNPN, 0x01, 0x04, 33}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_CMDERR, mockTransport->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_PARAM_IDX, mockTransport->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  assertEquals(OPC_RQNPN, mockTransport->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_PARAM_IDX, mockTransport->sent_messages[1].data[5]);
}

void testReadNodeParameterShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_RQNPN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_RQNPN, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

void testModuleName()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_RQMN}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NAME, mockTransport->sent_messages[0].data[0]);
  assertEquals(moduleName[0], mockTransport->sent_messages[0].data[1]);
  assertEquals(moduleName[1], mockTransport->sent_messages[0].data[2]);
  assertEquals(moduleName[2], mockTransport->sent_messages[0].data[3]);
  assertEquals(moduleName[3], mockTransport->sent_messages[0].data[4]);
  assertEquals(moduleName[4], mockTransport->sent_messages[0].data[5]);
  assertEquals(moduleName[5], mockTransport->sent_messages[0].data[6]);
  assertEquals(moduleName[6], mockTransport->sent_messages[0].data[7]);
}

void testModuleNameNormal()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_RQMN}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
}

void testHeartBeat()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setHeartBeat(true);

  // Not expecting a heartbeat before 5 seconds.
  addMillis(4500);
  controller.process();
  assertEquals(0, mockTransport->sent_messages.size());

  // But expecting a heartbeat after 5 seconds.
  addMillis(1000);
  controller.process();
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_HEARTB, mockTransport->sent_messages[0].data[0]);
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(5, mockTransport->sent_messages.size());

  assertEquals(OPC_SD, mockTransport->sent_messages[0].data[0]);
  assertEquals(4, mockTransport->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransport->sent_messages[1].data[0]);
  assertEquals(1, mockTransport->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[2].data[0]);
  assertEquals(2, mockTransport->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_CONSUMER, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[3].data[0]);
  assertEquals(3, mockTransport->sent_messages[3].data[3]); // index
  assertEquals(SERVICE_ID_PRODUCER, mockTransport->sent_messages[3].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[3].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[4].data[0]);
  assertEquals(4, mockTransport->sent_messages[4].data[3]); // index
  assertEquals(SERVICE_ID_STREAMING, mockTransport->sent_messages[4].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[4].data[5]); // version
}

void testServiceDiscoveryLongMessageSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 4}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(4, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_STREAMING, mockTransport->sent_messages[0].data[4]);
  // Not testing service data bytes.
}

void testServiceDiscoveryIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 7}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_RQSD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[0].data[4]);
  assertEquals(GRSP_INVALID_SERVICE, mockTransport->sent_messages[0].data[5]);
}

void testServiceDiscoveryShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 3, {OPC_RQSD, 0x01, 0x04}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_RQSD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

}

void testMinimumNodeService()
{
  testUninitializedRequestNodeNumber();
  testUninitializedRequestNodeNumberMissingSNN();
  testNormalRequestNodeNumber();
  testNormalRequestNodeNumberMissingSNN();
  testReleaseNodeNumberByUI();
  testRequestNodeNumberElsewhere();
  testSetNodeNumber();
  testSetNodeNumberNormal();
  testSetNodeNumberShort();
  testReadNodeParametersNormalMode();
  testReadNodeParametersSetupMode();
  testReadNodeParameterCount();
  testReadNodeParameterModuleId();
  testReadNodeParameterInvalidIndex();
  testReadNodeParameterShortMessage();
  testModuleName();
  testModuleNameNormal();
  testHeartBeat();
  testServiceDiscovery();
  testServiceDiscoveryLongMessageSvc();
  testServiceDiscoveryIndexOutOfBand();
  testServiceDiscoveryShortMessage();
}
