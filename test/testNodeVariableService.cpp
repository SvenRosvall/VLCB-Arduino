//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for NodeVariableService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "NodeVariableService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "MockTransportService.h"

namespace
{
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransportService.reset(new MockTransportService);

  static std::unique_ptr<VLCB::NodeVariableService> nodeVariableService;
  nodeVariableService.reset(new VLCB::NodeVariableService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), nodeVariableService.get(), mockTransportService.get()});
  controller.begin();

  return controller;
}

void testNumNVs()
{
  test();

  VLCB::Controller controller = createController();

  // read parameter 6 which stores number of NVs.
  VLCB::VlcbMessage msg = {4, {OPC_RQNPN, 0x01, 0x04, PAR_NVNUM}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PAR_NVNUM, mockTransportService->sent_messages[0].data[3]); // parameter index
  assertEquals(4, mockTransportService->sent_messages[0].data[4]); // parameter value
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(4, mockTransportService->sent_messages.size());

  assertEquals(OPC_SD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(3, mockTransportService->sent_messages[0].data[5]); // Number of services

  assertEquals(OPC_SD, mockTransportService->sent_messages[1].data[0]);
  assertEquals(1, mockTransportService->sent_messages[1].data[3]); // index
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[1].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[1].data[5]); // version

  assertEquals(OPC_SD, mockTransportService->sent_messages[2].data[0]);
  assertEquals(2, mockTransportService->sent_messages[2].data[3]); // index
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryNodeVarSvc()
{
  test();

  VLCB::Controller controller = createController();
  controller.getModuleConfig()->setNumNodeVariables(7);

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ESD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(2, mockTransportService->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]); // service ID
  
  assertEquals(7, mockTransportService->sent_messages[0].data[5]); // Number of node variables
}

void testSetAndReadNV()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 3 to 42
  VLCB::VlcbMessage msg = {5, {OPC_NVSET, 0x01, 0x04, 3, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);

  mockTransportService->clearMessages();

  // Read NV 3
  msg = {4, {OPC_NVRD, 0x01, 0x04, 3}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(3, mockTransportService->sent_messages[0].data[3]); // NV index
  assertEquals(42, mockTransportService->sent_messages[0].data[4]); // NV value
}

void testSetAndReadNVnew()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 3 to 17
  VLCB::VlcbMessage msg = {5, {OPC_NVSETRD, 0x01, 0x04, 3, 17}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(3, mockTransportService->sent_messages[0].data[3]); // NV index
  assertEquals(17, mockTransportService->sent_messages[0].data[4]); // NV value
}

void testSetNVIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {5, {OPC_NVSET, 0x01, 0x04, 7, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVSET, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[0].data[5]);

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[1].data[3]);
}

void testReadNVIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {4, {OPC_NVRD, 0x01, 0x04, 7}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVRD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[0].data[5]);

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[1].data[3]);
}

void testReadNVAll()
{
  test();

  VLCB::Controller controller = createController();
  for (int i = 1 ; i <= configuration->getNumNodeVariables() ; ++i)
  {
    configuration->writeNV(i, 20 + i);
  }

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {4, {OPC_NVRD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1 + configuration->getNumNodeVariables(), mockTransportService->sent_messages.size());

  // First response shall contain the number of NVs
  assertEquals(OPC_NVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]); // NV index
  assertEquals(configuration->getNumNodeVariables(), mockTransportService->sent_messages[0].data[4]); // NV value

  // The following are all the NVs
  for (int i = 1 ; i <= configuration->getNumNodeVariables() ; ++i)
  {
    assertEquals(OPC_NVANS, mockTransportService->sent_messages[i].data[0]);
    assertEquals(i, mockTransportService->sent_messages[i].data[3]); // NV index
    assertEquals(20 + i, mockTransportService->sent_messages[i].data[4]); // NV value
  }
}

void testSetNVnewIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {5, {OPC_NVSETRD, 0x01, 0x04, 7, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVSETRD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[0].data[5]);

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransportService->sent_messages[1].data[3]);
}

void testSetNVShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {4, {OPC_NVSET, 0x01, 0x04, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVSET, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testReadNVShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {3, {OPC_NVRD, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVRD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testSetNVnewShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::VlcbMessage msg = {4, {OPC_NVSETRD, 0x01, 0x04, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_NVSETRD, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

}

void testNodeVariableService()
{
  testNumNVs();
  testServiceDiscovery();
  testServiceDiscoveryNodeVarSvc();
  testSetAndReadNV();
  testSetAndReadNVnew();
  testReadNVAll();
  testSetNVIndexOutOfBand();
  testReadNVIndexOutOfBand();
  testSetNVnewIndexOutOfBand();
  testSetNVShortMessage();
  testReadNVShortMessage();
  testSetNVnewShortMessage();
}
