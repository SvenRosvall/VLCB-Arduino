//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

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

void testServiceDiscovery()
{
  test();

  MockUserInterface mockUserInterface;
  MockStorage mockStorage;
  VLCB::Configuration configuration(&mockStorage);
  MockTransport mockTransport;
  VLCB::MinimumNodeService minimumNodeService;
  VLCB::LongMessageService longMessageService;
  VLCB::CbusService cbusService;
  VLCB::Controller controller(&mockUserInterface, &configuration, &mockTransport,
                              {&minimumNodeService, &longMessageService, &cbusService});

  configuration.EE_NVS_START = 0;
  configuration.EE_NUM_NVS = 4;
  configuration.EE_EVENTS_START = 20;
  configuration.EE_MAX_EVENTS = 20;
  configuration.EE_NUM_NVS = 2;
  configuration.begin();
  // Not storing these in mockStorage. Setting directly instead.
  configuration.currentMode = VLCB::MODE_NORMAL;
  configuration.nodeNum = 0x0104;
  VLCB::Parameters params(configuration);
  params.setVersion(1, 1, 'a');
  params.setModuleId(253);
  params.setFlags(PF_FLiM | PF_BOOT);

  // assign to Controller
  controller.setParams(params.getParams());
  unsigned char moduleName[] = {'t', 'e', 's', 't', 'i', 'n', 'g', '\0'};
  controller.setName(moduleName);

  VLCB::CANFrame msg_rqsd = {0x11, false, false, 4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransport.setNextMessage(msg_rqsd);

  controller.process();

  // Verify sent messages.
  assertEquals(4, mockTransport.sent_messages.size());

  assertEquals(OPC_SD, mockTransport.sent_messages[0].data[0]);
  assertEquals(3, mockTransport.sent_messages[0].data[3]); // Number of services

  assertEquals(OPC_SD, mockTransport.sent_messages[1].data[0]);
  assertEquals(0, mockTransport.sent_messages[1].data[1]); // index
  assertEquals(1, mockTransport.sent_messages[1].data[2]); // service ID
  assertEquals(1, mockTransport.sent_messages[1].data[3]); // version

  assertEquals(OPC_SD, mockTransport.sent_messages[2].data[0]);
  assertEquals(2, mockTransport.sent_messages[1].data[1]); // index
  assertEquals(17, mockTransport.sent_messages[1].data[2]); // service ID
  assertEquals(1, mockTransport.sent_messages[1].data[3]); // version

  assertEquals(OPC_SD, mockTransport.sent_messages[3].data[0]);
  assertEquals(3, mockTransport.sent_messages[1].data[1]); // index
  // Don't care about details of CbusService.
}

}

void testMinimumNodeService()
{
  testServiceDiscovery();
}