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

namespace
{

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  static std::unique_ptr<VLCB::NodeVariableService> nodeVariableService;
  nodeVariableService.reset(new VLCB::NodeVariableService);

  return ::createController({minimumNodeService.get(), nodeVariableService.get()});
}

void testNumNVs()
{
  test();

  VLCB::Controller controller = createController();

  // read parameter 6 which stores number of NVs.
  VLCB::CANFrame msg = {0x11, 4, {OPC_RQNPN, 0x01, 0x04, 6}};
  mockTransport->setNextMessage(msg);

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

  VLCB::CANFrame msg = {0x11, 4, {OPC_RQSD, 0x01, 0x04, 0}};
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
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryNodeVarSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::CANFrame msg = {0x11, 4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testSetAndReadNV()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 3 to 42
  VLCB::CANFrame msg = {0x11, 5, {OPC_NVSET, 0x01, 0x04, 3, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  
  mockTransport->clearMessages();
  
  // Read NV 3
  msg = {0x11, 4, {OPC_NVRD, 0x01, 0x04, 3}};
  mockTransport->setNextMessage(msg);

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
  VLCB::CANFrame msg = {0x11, 5, {OPC_NVSETRD, 0x01, 0x04, 3, 17}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(3, mockTransport->sent_messages[0].data[3]); // NV index
  assertEquals(17, mockTransport->sent_messages[0].data[4]); // NV value
}

void testSetNVIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 5, {OPC_NVSET, 0x01, 0x04, 7, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVSET, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[0].data[5]);
  
  assertEquals(OPC_CMDERR, mockTransport->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[1].data[3]);
}

void testReadNVIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 4, {OPC_NVRD, 0x01, 0x04, 7}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVRD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[0].data[5]);
  
  assertEquals(OPC_CMDERR, mockTransport->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[1].data[3]);
}

void testReadNVAll()
{
  test();

  VLCB::Controller controller = createController();
  for (int i = 1 ; i <= configuration->EE_NUM_NVS ; ++i)
  {
    configuration->writeNV(i, 20 + i);
  }
  
  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 4, {OPC_NVRD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1 + configuration->EE_NUM_NVS, mockTransport->sent_messages.size());

  // First response shall contain the number of NVs
  assertEquals(OPC_NVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[3]); // NV index
  assertEquals(configuration->EE_NUM_NVS, mockTransport->sent_messages[0].data[4]); // NV value

  // The following are all the NVs
  for (int i = 1 ; i <= configuration->EE_NUM_NVS ; ++i)
  {
    assertEquals(OPC_NVANS, mockTransport->sent_messages[i].data[0]);
    assertEquals(i, mockTransport->sent_messages[i].data[3]); // NV index
    assertEquals(20 + i, mockTransport->sent_messages[i].data[4]); // NV value
  }
}

void testSetNVnewIndexOutOfBand()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 5, {OPC_NVSETRD, 0x01, 0x04, 7, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVSETRD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[0].data[5]);
  
  assertEquals(OPC_CMDERR, mockTransport->sent_messages[1].data[0]);
  assertEquals(CMDERR_INV_NV_IDX, mockTransport->sent_messages[1].data[3]);
}

void testSetNVShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 4, {OPC_NVSET, 0x01, 0x04, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVSET, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

void testReadNVShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 3, {OPC_NVRD, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVRD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

void testSetNVnewShortMessage()
{
  test();

  VLCB::Controller controller = createController();

  // Set NV 7 to 42
  VLCB::CANFrame msg = {0x11, 4, {OPC_NVSETRD, 0x01, 0x04, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_NVSETRD, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_NV, mockTransport->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
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
