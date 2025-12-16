//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// Test cases for EventTeachingService.

#include <memory>
#include "TestTools.hpp"
#include "Controller.h"
#include "MinimumNodeService.h"
#include "EventTeachingService.h"
#include "Parameters.h"
#include "VlcbCommon.h"
#include "MockTransportService.h"

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;
static std::unique_ptr<MockTransportService> mockTransportService;

VLCB::Controller createController()
{
  minimumNodeService.reset(new VLCB::MinimumNodeService);

  mockTransportService.reset(new MockTransportService);

  static std::unique_ptr<VLCB::EventTeachingService> eventTeachingService;
  eventTeachingService.reset(new VLCB::EventTeachingService);

  VLCB::Controller controller = ::createController({minimumNodeService.get(), eventTeachingService.get(), mockTransportService.get()});
  controller.begin();

  return controller;
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
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[2].data[4]); // service ID
  assertEquals(1, mockTransportService->sent_messages[2].data[5]); // version
}

void testServiceDiscoveryEventProdSvc()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_RQSD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ESD, mockTransportService->sent_messages[0].data[0]);
  assertEquals(2, mockTransportService->sent_messages[0].data[3]); // index
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]); // service ID
  // Not testing service data bytes.
}

void testEventSlotsLeftAtStart()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {3, {OPC_NNEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVNLF, mockTransportService->sent_messages[0].data[0]);
  assertEquals(20, mockTransportService->sent_messages[0].data[3]);  
}

void testEventsStoredAtStart()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Verify sent messages.
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);  
}

void testEnterLearnModeOld()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(8, mockTransportService->sent_messages[0].data[3]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[4] & PF_LRN);
  mockTransportService->clearMessages();
  
  // Send NNULN
  msg = {3, {OPC_NNULN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  VLCB::VlcbMessage msg2 = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg2);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransportService->setNextMessage(msg3);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(8, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0, mockTransportService->sent_messages[0].data[4] & PF_LRN);
  mockTransportService->clearMessages();
}

void testEnterLearnModeViaMode()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(8, mockTransportService->sent_messages[0].data[3]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[4] & PF_LRN);
  mockTransportService->clearMessages();
  
  // Leave Learn mode
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  VLCB::VlcbMessage msg2 = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg2);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransportService->setNextMessage(msg3);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(8, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0, mockTransportService->sent_messages[0].data[4] & PF_LRN);
  mockTransportService->clearMessages();
}

void testEnterLearnModeForOtherNode()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x05, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  // Learn flag shall not be set.
  assertEquals(0, mockTransportService->sent_messages[0].data[5] & PF_LRN);
}

void testTeachEvent()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Teach an event
  // Data: OP, NN, EN, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify the event variable 1
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[4]);
  assertEquals(1, mockTransportService->sent_messages[0].data[5]);
  assertEquals(42, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
  
  // Verify all event variables
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(3, mockTransportService->sent_messages.size());

  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5]);
  assertEquals(2, mockTransportService->sent_messages[0].data[6]);
  
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[1].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[1].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[1].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[1].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[1].data[4]);
  assertEquals(1, mockTransportService->sent_messages[1].data[5]);
  assertEquals(42, mockTransportService->sent_messages[1].data[6]);

  assertEquals(OPC_EVANS, mockTransportService->sent_messages[2].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[2].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[2].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[2].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[2].data[4]);
  assertEquals(2, mockTransportService->sent_messages[2].data[5]);
  // We haven't taught EV2 so don't care about its value.
  mockTransportService->clearMessages();

  // Finished learning
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify there is an event.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransportService->sent_messages[0].data[0]);
  assertEquals(1, mockTransportService->sent_messages[0].data[3]);
  mockTransportService->clearMessages();

  // Verify the contents of this event.
  // Note: CBUS lib does not implement OPC_NENRD.
  // Data: OP, NN, Event index
  msg = {4, {OPC_NENRD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ENRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[5]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
  
  // Verify the event variable 1
  // Data: OP, NN, Event index, EV#
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  assertEquals(1, mockTransportService->sent_messages[0].data[4]);
  assertEquals(42, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();
}

void testTeachEventIndexedAndClear()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify the event variable 1
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[4]);
  assertEquals(1, mockTransportService->sent_messages[0].data[5]);
  assertEquals(42, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
  
  // Clear all events.
  // Data: OP, NN, EN
  msg = {3, {OPC_NNCLR, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify there are no events.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  mockTransportService->clearMessages();
}

void testTeachEventIndexedWithNullNNEN()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Teach an event with null NN/EN
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0, 0, 0, 0, 0, 1, 43}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify the event variable 1
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[4]);
  assertEquals(1, mockTransportService->sent_messages[0].data[5]);
  assertEquals(43, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
}

void testTeachEventIndexedWithEV0()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Teach an event with null NN/EN
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x04, 0x06, 0x05, 0x08, 0, 0, 17}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Read event at index 0
  // Data: OP, NN, Event index
  msg = {4, { OPC_NENRD, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ENRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x01, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x04, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[5]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[6]);
  assertEquals(0, mockTransportService->sent_messages[0].data[7]);
  mockTransportService->clearMessages();
}

void testLearnEventIndexedDelete()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify the event variable 1
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[4]);
  assertEquals(1, mockTransportService->sent_messages[0].data[5]);
  assertEquals(42, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
  
  // Delete the event. Do this with EV#==0 and EV value==0
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 0, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify there are no events.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  mockTransportService->clearMessages();
}

void testEventHashCollisionAndUnlearn()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();
  
  // Teach second event with same hash.
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x08, 0x07, 1, 43}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Verify the second event, variable 1. Ensure we can get it despite duplicate hash
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x08, 0x07, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[2]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[4]);
  assertEquals(1, mockTransportService->sent_messages[0].data[5]);
  assertEquals(43, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();

  // Finished learning
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  
  // Verify there are two events now.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransportService->sent_messages[0].data[0]);
  assertEquals(2, mockTransportService->sent_messages[0].data[3]);
  mockTransportService->clearMessages();

  // Learn mode
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Unlearn the second event
  msg = {5, {OPC_EVULN, 0x05, 0x06, 0x08, 0x07}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Leave Learn mode
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify events left.
  // Data: OP, NN, Event index
  msg = {3, {OPC_NERD, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_ENRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0x06, mockTransportService->sent_messages[0].data[4]);
  assertEquals(0x07, mockTransportService->sent_messages[0].data[5]);
  assertEquals(0x08, mockTransportService->sent_messages[0].data[6]);
  mockTransportService->clearMessages();
}

void testIgnoreMsgsForOtherNodes()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN to other node
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify parameter learn set.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // Send NNCLR to other node
  msg = {3, {OPC_NNCLR, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send NNEVN to other node
  msg = {3, {OPC_NNEVN, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send NERD to other node
  msg = {3, {OPC_NERD, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send RQEVN to other node
  msg = {3, {OPC_RQEVN, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());

  // Send NENRD to other node
  msg = {4, {OPC_NENRD, 0x01, 0x05, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send REVAL to other node
  msg = {4, {OPC_REVAL, 0x01, 0x05, 0,1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  // Expect no response or error messages.
  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();
}

void testIgnoreUnlearnForOtherNodes()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify parameter learn set.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // Send NNULN to other node
  msg = {3, {OPC_NNULN, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify node is still in learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[5] & PF_LRN);
}

void testIgnoreIfNotInLearnMode()
{
  test();

  VLCB::Controller controller = createController();

  // Send EVULN
  VLCB::VlcbMessage msg = {5, {OPC_EVULN, 0x05, 0x06, 0x08, 0x07}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();
  
  // Send EVLRN
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send EVLRNI
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();

  // Send REQEV
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());
  mockTransportService->clearMessages();
}

void testUpdateProducedEventNNEN()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Add another event with same EV#1 value. This is OK in library version 2.xx.
  msg = {7, {OPC_EVLRN, 0x01, 0x06, 0x01, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
}

void testUpdateProducedEventNNENToExistingEvent()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Teach a second event
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x09, 1, 43}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Update the EV#1 of second event to match the first event. This is OK in library version 2.xx.
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x09, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
}

void testEnterLearnModeOldOtherNode()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransportService->sent_messages[0].data[5] & PF_LRN);
  mockTransportService->clearMessages();

  // Send NNLRN for another node
   msg = {3, {OPC_NNLRN, 0x01, 0x05}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify parameter learn set.
  // Send QNN - PNN response contains bit 5 as learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_PNN, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[5] & PF_LRN);
}

void testEnterLearnModeViaModeInSetup()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransportService->sent_messages[0].data[4]); // Yes, filtering is done in MNS.
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
}

void testClearEventsNotLearnMode()
{
  test();

  VLCB::Controller controller = createController();
  
  VLCB::VlcbMessage msg = {3, {OPC_NNCLR, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_NOT_LRN, mockTransportService->sent_messages[0].data[3]);
  
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_NNCLR, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_NOT_LRN, mockTransportService->sent_messages[1].data[5]);
}

void testNenrdWithBadIndex()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_NENRD, 0x01, 0x04, 30}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_NENRD, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[1].data[5]);
}

void testNenrdForEmptyIndex()
{
  test();

  VLCB::Controller controller = createController();

  VLCB::VlcbMessage msg = {4, {OPC_NENRD, 0x01, 0x04, 2}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_NENRD, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[1].data[5]);
}

void testEvulnErrors()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Short message
  msg = {4, {OPC_EVULN, 0x05, 0x06, 0x08}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_EVULN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // Unlearn unknown event
  msg = {5, {OPC_EVULN, 0x05, 0x06, 0x08, 0x17}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INVALID_EVENT, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_EVULN, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INVALID_EVENT, mockTransportService->sent_messages[1].data[5]);
}

void testReval()
{
  test();

  VLCB::Controller controller = createController();
  
  // Create one event.
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(0, mockTransportService->sent_messages.size());

  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 17}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 2, 42}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(0, mockTransportService->sent_messages.size());

  // Verify count of event variables.
  // Data: OP, NN, Event index, EV#
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 0}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(3, mockTransportService->sent_messages.size());
  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  assertEquals(0, mockTransportService->sent_messages[0].data[4]);
  assertEquals(2, mockTransportService->sent_messages[0].data[5]);

  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[1].data[0]);
  assertEquals(0, mockTransportService->sent_messages[1].data[3]);
  assertEquals(1, mockTransportService->sent_messages[1].data[4]);
  assertEquals(17, mockTransportService->sent_messages[1].data[5]);

  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[2].data[3]);
  assertEquals(2, mockTransportService->sent_messages[2].data[4]);
  assertEquals(42, mockTransportService->sent_messages[2].data[5]);
  mockTransportService->clearMessages();

  // Verify the event variable 1
  // Data: OP, NN, Event index, EV#
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  assertEquals(1, mockTransportService->sent_messages[0].data[4]);
  assertEquals(17, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // Verify the event variable 2
  // Data: OP, NN, Event index, EV#
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 2}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(1, mockTransportService->sent_messages.size());
  assertEquals(OPC_NEVAL, mockTransportService->sent_messages[0].data[0]);
  assertEquals(0, mockTransportService->sent_messages[0].data[3]);
  assertEquals(2, mockTransportService->sent_messages[0].data[4]);
  assertEquals(42, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();
}

void testRevalErrors()
{
  test();

  VLCB::Controller controller = createController();

  // Short message
  VLCB::VlcbMessage msg = {4, {OPC_REVAL, 0x01, 0x04, 0}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_REVAL, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // Event index out of range
  msg = {5, {OPC_REVAL, 0x01, 0x04, 20, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_REVAL, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();

  // Missing event - none taught yet.
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_REVAL, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();

  // Teach an event for following tests
  msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransportService->setNextMessage(msg);
  process(controller);
  assertEquals(0, mockTransportService->sent_messages.size());

  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // Event variable out of range
  msg = {5, {OPC_REVAL, 0x01, 0x04, 0, 3}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_REVAL, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();

  // Note: Opcode spec says to check for EV not set for event. Doesn't apply in this implementation
}

void testReqevErrors()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Short message
  msg = {5, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_REQEV, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // No such event
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INVALID_EVENT, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_REQEV, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INVALID_EVENT, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();

  // Teach an event for following tests
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  mockTransportService->clearMessages();

  // EV# too large
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 3}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_REQEV, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();
}

void testLearnErrors()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Short message
  msg = {6, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_EVLRN, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // EV index out of range
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 3, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_EVLRN, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_INV_EV_IDX, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();

  // Event table full
  for (byte i = 1 ; i <= configuration->getNumEvents() ; ++i)
  {
    msg = {7, {OPC_EVLRN, 0x05, 0x06, 0, i, 1, i}};
    mockTransportService->setNextMessage(msg);

    process(controller);

    assertEquals(2, mockTransportService->sent_messages.size());
    assertEquals(OPC_WRACK, mockTransportService->sent_messages[0].data[0]);
    mockTransportService->clearMessages();
  }
  

  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(2, mockTransportService->sent_messages.size());

  assertEquals(OPC_CMDERR, mockTransportService->sent_messages[0].data[0]);
  assertEquals(CMDERR_TOO_MANY_EVENTS, mockTransportService->sent_messages[0].data[3]);

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[1].data[0]);
  assertEquals(OPC_EVLRN, mockTransportService->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[1].data[4]);
  assertEquals(CMDERR_TOO_MANY_EVENTS, mockTransportService->sent_messages[1].data[5]);
  mockTransportService->clearMessages();
}

void testLearnIndexErrors()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(0, mockTransportService->sent_messages.size());

  // Short message
  msg = {7, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_EVLRNI, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_CMD, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();

  // Event index out of range
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 20, 1, 42}};
  mockTransportService->setNextMessage(msg);

  process(controller);

  assertEquals(1, mockTransportService->sent_messages.size());

  assertEquals(OPC_GRSP, mockTransportService->sent_messages[0].data[0]);
  assertEquals(OPC_EVLRNI, mockTransportService->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransportService->sent_messages[0].data[4]);
  assertEquals(CMDERR_INV_EN_IDX, mockTransportService->sent_messages[0].data[5]);
  mockTransportService->clearMessages();
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
  testEnterLearnModeForOtherNode();
  testTeachEvent();
  testTeachEventIndexedAndClear();
  testTeachEventIndexedWithNullNNEN(); // updates EV for a slot
  testTeachEventIndexedWithEV0(); // Teach event without an EV.
  testLearnEventIndexedDelete(); // Delete an event slot.
  testEventHashCollisionAndUnlearn(); // tests event lookup in Configuration::findExistingEvent()
  testUpdateProducedEventNNEN();
  testUpdateProducedEventNNENToExistingEvent();

  // test error conditions.
  testEnterLearnModeOldOtherNode();
  testEnterLearnModeViaModeInSetup();
  testClearEventsNotLearnMode();
  testIgnoreMsgsForOtherNodes();
  testIgnoreUnlearnForOtherNodes();
  testIgnoreIfNotInLearnMode();
  testNenrdWithBadIndex();
  testNenrdForEmptyIndex();
  testEvulnErrors();
  testReval();
  testRevalErrors();
  testReqevErrors();
  testLearnErrors();
  testLearnIndexErrors();
}
