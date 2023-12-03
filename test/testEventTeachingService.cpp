//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for EventTeachingService.

// TODO: 
//NNLRN - Done
//NNULN - Done
//NNCLR
//NNEVN - Done
//NERD (Read all events)
//RQEVN - Done
//NENRD (Read Event by Index)
//EVULN
//REVAL (Request EV Read)
//REQEV (Read Event Variable)
//EVLRN (Teach an EV)
//EVLRNI

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "EventTeachingService.h"
#include "Parameters.h"
#include "VlcbCommon.h"

namespace
{

VLCB::Controller createController()
{
  static std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  static std::unique_ptr<VLCB::EventTeachingService> eventTeachingService;
  eventTeachingService.reset(new VLCB::EventTeachingService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), eventTeachingService.get()});
  controller.begin();

  return controller;
}

void testServiceDiscovery()
{
  test();

  VLCB::Controller controller = createController();

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
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransport->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransport->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ESD, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransport->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testEventSlotsLeftAtStart()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {3, {OPC_NNEVN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_EVNLF, mockTransport->sent_messages[0].data[0]);
  assertEquals(20, mockTransport->sent_messages[0].data[3]);  
}

void testEventsStoredAtStart()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Verify sent messages.
  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[3]);  
}

void testEnterLearnModeOld()
{
  const int LEARN_BIT = 1 << 5;
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(LEARN_BIT, mockTransport->sent_messages[0].data[5] & LEARN_BIT);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(LEARN_BIT, mockTransport->sent_messages[0].data[4] & LEARN_BIT);
  mockTransport->clearMessages();
  
  // Send NNULN
  msg = {3, {OPC_NNULN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  VLCB::VlcbMessage msg2 = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg2);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[5] & LEARN_BIT);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg3);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(0, mockTransport->sent_messages[0].data[4] & LEARN_BIT);
  mockTransport->clearMessages();
}

void testEnterLearnModeViaMode()
{
  const int LEARN_BIT = 1 << 5;
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(LEARN_BIT, mockTransport->sent_messages[0].data[5] & LEARN_BIT);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(LEARN_BIT, mockTransport->sent_messages[0].data[4] & LEARN_BIT);
  mockTransport->clearMessages();
  
  // Send NNULN
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  VLCB::VlcbMessage msg2 = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg2);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[5] & LEARN_BIT);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg3);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(0, mockTransport->sent_messages[0].data[4] & LEARN_BIT);
  mockTransport->clearMessages();
}

void testTeachEvent()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Teach an event
  // Data: OP, NN, EN, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  mockTransport->clearMessages();

  // Verify the event variable 1
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransport->sent_messages[0].data[4]);
  assertEquals(1, mockTransport->sent_messages[0].data[5]);
  assertEquals(42, mockTransport->sent_messages[0].data[6]);
  mockTransport->clearMessages();
  
  // Verify all event variables
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 0}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(3, mockTransport->sent_messages.size());

  assertEquals(OPC_EVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransport->sent_messages[0].data[4]);
  assertEquals(0, mockTransport->sent_messages[0].data[5]);
  assertEquals(2, mockTransport->sent_messages[0].data[6]);
  
  assertEquals(OPC_EVANS, mockTransport->sent_messages[1].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[1].data[1]);
  assertEquals(0x06, mockTransport->sent_messages[1].data[2]);
  assertEquals(0x07, mockTransport->sent_messages[1].data[3]);
  assertEquals(0x08, mockTransport->sent_messages[1].data[4]);
  assertEquals(1, mockTransport->sent_messages[1].data[5]);
  assertEquals(42, mockTransport->sent_messages[1].data[6]);

  assertEquals(OPC_EVANS, mockTransport->sent_messages[2].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[2].data[1]);
  assertEquals(0x06, mockTransport->sent_messages[2].data[2]);
  assertEquals(0x07, mockTransport->sent_messages[2].data[3]);
  assertEquals(0x08, mockTransport->sent_messages[2].data[4]);
  assertEquals(2, mockTransport->sent_messages[2].data[5]);
  // We haven't taught EV2 so don't care about its value.
  mockTransport->clearMessages();

  // Finished learning
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Verify there is an event.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransport->sent_messages[0].data[0]);
  assertEquals(1, mockTransport->sent_messages[0].data[3]);
  mockTransport->clearMessages();

  // Verify the contents of this event.
  // Note: CBUS lib does not implement OPC_NENRD.
  // Data: OP, NN, Event index
  msg = {4, {OPC_NENRD, 0x01, 0x04, 0}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ENRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x06, mockTransport->sent_messages[0].data[4]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[5]);
  assertEquals(0x08, mockTransport->sent_messages[0].data[6]);
  mockTransport->clearMessages();
  
  // Verify the event variable 1
  // Data: OP, NN, Event index, EV#
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NEVAL, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[3]);
  assertEquals(1, mockTransport->sent_messages[0].data[4]);
  assertEquals(42, mockTransport->sent_messages[0].data[5]);
  mockTransport->clearMessages();
}

}

void testEventTeachingService()
{
  testServiceDiscovery();
  testServiceDiscoveryEventProdSvc();
  testEventSlotsLeftAtStart();
  testEventsStoredAtStart();
  testEnterLearnModeOld();
  testEnterLearnModeViaMode();
  testTeachEvent();
}
