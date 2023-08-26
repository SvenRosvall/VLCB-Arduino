//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for NodeVariablesService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MockUserInterface.h"
#include "MinimumNodeService.h"
#include "NodeVariablesService.h"
#include "MockTransport.h"
#include "MockStorage.h"
#include "vlcbdefs.hpp"
#include "Parameters.h"
#include "LongMessageService.h"

namespace
{
static std::unique_ptr<MockUserInterface> mockUserInterface;
static std::unique_ptr<MockTransport> mockTransport;
static std::unique_ptr<VLCB::Configuration> configuration;

VLCB::Controller createController()
{
  // Use pointers to objects to create the controller with.
  // Use unique_ptr so that next invocation deletes the previous objects.
  mockTransport.reset(new MockTransport);
  mockUserInterface.reset(new MockUserInterface);
  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  configuration.reset(new VLCB::Configuration(mockStorage.get()));
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);
  static std::unique_ptr<VLCB::NodeVariablesService> NodeVariablesService;
  NodeVariablesService.reset(new VLCB::NodeVariablesService);
  VLCB::Controller controller(mockUserInterface.get(), configuration.get(), mockTransport.get(),
                              {minimumNodeService.get(), NodeVariablesService.get()});

  configuration->EE_NVS_START = 0;
  configuration->EE_NUM_NVS = 4;
  configuration->EE_EVENTS_START = 20;
  configuration->EE_MAX_EVENTS = 20;
  configuration->EE_NUM_EVS = 2;
  configuration->begin();
  // Not storing these in mockStorage. Setting directly instead.
  configuration->currentMode = VLCB::MODE_NORMAL;
  configuration->nodeNum = 0x0104;
  static std::unique_ptr<VLCB::Parameters> params;
  params.reset(new VLCB::Parameters(*configuration));
  params->setVersion(1, 1, 'a');
  params->setModuleId(253);
  params->setFlags(PF_FLiM | PF_BOOT);

  // assign to Controller
  controller.setParams(params->getParams());
  unsigned char moduleName[] = {'t', 'e', 's', 't', 'i', 'n', 'g', '\0'};
  controller.setName(moduleName);
  return controller;
}

void testNumNVs()
{
  test();

  VLCB::Controller controller = createController();

  // read parameter 6 which stores number of NVs.
  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQNPN, 0x01, 0x04, 6}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(6, mockTransport->sent_messages[0].data[3]); // parameter index
  assertEquals(4, mockTransport->sent_messages[0].data[4]); // parameter value
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(3, mockTransport->sent_messages.size());

  assertEquals(OPC_SD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransport->sent_messages[1].data[0]);
  assertEquals(1, mockTransport->sent_messages[1].data[3]); // index
  assertEquals(1, mockTransport->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransport->sent_messages[2].data[0]);
  assertEquals(2, mockTransport->sent_messages[2].data[3]); // index
  assertEquals(2, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryNodeVarSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(2, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testSetAndReadNV()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 3 to 42
  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_NVSET, 0x01, 0x04, 3, 42}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  
  mockTransport->clearMessages();
  
  // Read NV 3
  msg_rqsd = {0x11, false, false, 4, {OPC_NVRD, 0x01, 0x04, 3}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(3, mockTransport->sent_messages[0].data[3]); // NV index
  assertEquals(42, mockTransport->sent_messages[0].data[4]); // NV value
}

void testSetAndReadNVnew()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 3 to 17
  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_NVSETRD, 0x01, 0x04, 3, 17}};
  mockTransport->setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(3, mockTransport->sent_messages[0].data[3]); // NV index
  assertEquals(17, mockTransport->sent_messages[0].data[4]); // NV value
}

}

void testNodeVariablesService()
{
  testNumNVs();
  testServiceDiscovery();
  testServiceDiscoveryNodeVarSvc();
  testSetAndReadNV();
  testSetAndReadNVnew();
}
