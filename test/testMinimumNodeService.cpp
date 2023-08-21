//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

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
#include "CbusService.h"

namespace
{

VLCB::Controller createController(MockTransport &mockTransport)
{
  // Use pointers to objects to create the controller with.
  // Use static unique_ptr so that next invocation deletes the previous objects.
  static std::unique_ptr<MockUserInterface> mockUserInterface;
  mockUserInterface.reset(new MockUserInterface);
  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  static std::unique_ptr<VLCB::Configuration> configuration;
  configuration.reset(new VLCB::Configuration(mockStorage.get()));
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);
  static std::unique_ptr<VLCB::LongMessageService> longMessageService;
  longMessageService.reset(new VLCB::LongMessageService);
  static std::unique_ptr<VLCB::CbusService> cbusService;
  cbusService.reset(new VLCB::CbusService);
  VLCB::Controller controller(mockUserInterface.get(), configuration.get(), &mockTransport,
                              {minimumNodeService.get(), longMessageService.get(), cbusService.get()});

  configuration->EE_NVS_START = 0;
  configuration->EE_NUM_NVS = 4;
  configuration->EE_EVENTS_START = 20;
  configuration->EE_MAX_EVENTS = 20;
  configuration->EE_NUM_NVS = 2;
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

void testServiceDiscovery()
{
  test();

  MockTransport mockTransport;
  VLCB::Controller controller = createController(mockTransport);

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport.setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(4, mockTransport.sent_messages.size());

  assertEquals(OPC_SD, mockTransport.sent_messages[0].data[0]);
  assertEquals(3, mockTransport.sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransport.sent_messages[1].data[0]);
  assertEquals(1, mockTransport.sent_messages[1].data[3]); // index
  assertEquals(1, mockTransport.sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransport.sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransport.sent_messages[2].data[0]);
  assertEquals(2, mockTransport.sent_messages[2].data[3]); // index
  assertEquals(17, mockTransport.sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport.sent_messages[2].data[5]); // version

  assertEquals(OPC_SD, mockTransport.sent_messages[3].data[0]);
  assertEquals(3, mockTransport.sent_messages[3].data[3]); // index
  // Don't care about details of CbusService.
}

void testServiceDiscoveryLongMessageSvc()
{
  test();

  MockTransport mockTransport;
  VLCB::Controller controller = createController(mockTransport);

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport.setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport.sent_messages.size());
  assertEquals(OPC_ESD, mockTransport.sent_messages[0].data[0]);
  assertEquals(2, mockTransport.sent_messages[0].data[3]); // index
  assertEquals(17, mockTransport.sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

}

void testMinimumNodeService()
{
  testServiceDiscovery();
  testServiceDiscoveryLongMessageSvc();
}