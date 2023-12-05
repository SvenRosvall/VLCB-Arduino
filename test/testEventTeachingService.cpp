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

namespace
{
std::unique_ptr<VLCB::MinimumNodeService> minimumNodeService;

VLCB::Controller createController()
{
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
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[4] & PF_LRN);
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
  assertEquals(0, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg3);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(0, mockTransport->sent_messages[0].data[4] & PF_LRN);
  mockTransport->clearMessages();
}

void testEnterLearnModeViaMode()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
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
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  msg = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[4] & PF_LRN);
  mockTransport->clearMessages();
  
  // Leave Learn mode
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
  assertEquals(0, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // or RQNPN->PARAN : PARAM[8] bit 6
  VLCB::VlcbMessage msg3 = {4, {OPC_RQNPN, 0x01, 0x04, 8}};
  mockTransport->setNextMessage(msg3);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PARAN, mockTransport->sent_messages[0].data[0]);
  assertEquals(8, mockTransport->sent_messages[0].data[3]);
  assertEquals(0, mockTransport->sent_messages[0].data[4] & PF_LRN);
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

void testTeachEventIndexedAndClear()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
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
  
  // Clear all events.
  // Data: OP, NN, EN
  msg = {3, {OPC_NNCLR, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  mockTransport->clearMessages();

  // Verify there are no events.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[3]);
  mockTransport->clearMessages();
}

void testEventHashCollisionAndUnlearn()
{
  test();

  VLCB::Controller controller = createController();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Teach an event
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  mockTransport->clearMessages();
  
  // Teach second event with same hash.
  // Data: OP, NN, EN, Event index, EV#, EV Value
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x08, 0x07, 1, 43}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  mockTransport->clearMessages();

  // Verify the second event, variable 1. Ensure we can get it despite duplicate hash
  // Note: CBUS lib does not implement OPC_REQEV.
  // Data: OP, NN, EN, EV#
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x08, 0x07, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_EVANS, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[1]);
  assertEquals(0x06, mockTransport->sent_messages[0].data[2]);
  assertEquals(0x08, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[4]);
  assertEquals(1, mockTransport->sent_messages[0].data[5]);
  assertEquals(43, mockTransport->sent_messages[0].data[6]);
  mockTransport->clearMessages();

  // Finished learning
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  
  // Verify there are two events now.
  msg = {3, {OPC_RQEVN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_NUMEV, mockTransport->sent_messages[0].data[0]);
  assertEquals(2, mockTransport->sent_messages[0].data[3]);
  mockTransport->clearMessages();

  // Learn mode
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Unlearn the second event
  msg = {5, {OPC_EVULN, 0x05, 0x06, 0x08, 0x07}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_WRACK, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  mockTransport->clearMessages();

  // Leave Learn mode
  msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_OFF}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Verify events left.
  // Data: OP, NN, Event index
  msg = {3, {OPC_NERD, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_ENRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(0x05, mockTransport->sent_messages[0].data[3]);
  assertEquals(0x06, mockTransport->sent_messages[0].data[4]);
  assertEquals(0x07, mockTransport->sent_messages[0].data[5]);
  assertEquals(0x08, mockTransport->sent_messages[0].data[6]);
  mockTransport->clearMessages();
}

void testIgnoreMsgsForOtherNodes()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN to other node
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Verify parameter learn set.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(0, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // Send NNCLR to other node
  msg = {3, {OPC_NNCLR, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send NNEVN to other node
  msg = {3, {OPC_NNEVN, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send NERD to other node
  msg = {3, {OPC_NERD, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send RQEVN to other node
  msg = {3, {OPC_RQEVN, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());

  // Send NENRD to other node
  msg = {4, {OPC_NENRD, 0x01, 0x05, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send REVAL to other node
  msg = {4, {OPC_REVAL, 0x01, 0x05, 0,1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  // Expect no response or error messages.
  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();
}

void testIgnoreUnlearnForOtherNodes()
{
  test();

  VLCB::Controller controller = createController();

  // Send NNLRN
  VLCB::VlcbMessage msg = {3, {OPC_NNLRN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Verify parameter learn set.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // Send NNULN to other node
  msg = {3, {OPC_NNULN, 0x01, 0x05}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());

  // Verify node is still in learn mode.
  msg = {3, {OPC_QNN, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_PNN, mockTransport->sent_messages[0].data[0]);
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[5] & PF_LRN);
}

void testIgnoreIfNotInLearnMode()
{
  test();

  VLCB::Controller controller = createController();

  // Send EVULN
  VLCB::VlcbMessage msg = {5, {OPC_EVULN, 0x05, 0x06, 0x08, 0x07}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();
  
  // Send EVLRN
  msg = {7, {OPC_EVLRN, 0x05, 0x06, 0x07, 0x08, 1, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send EVLRNI
  msg = {8, {OPC_EVLRNI, 0x05, 0x06, 0x07, 0x08, 0, 1, 42}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();

  // Send REQEV
  msg = {6, {OPC_REQEV, 0x05, 0x06, 0x07, 0x08, 1}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(0, mockTransport->sent_messages.size());
  mockTransport->clearMessages();
}

void testEnterLearnModeOldOtherNode()
{
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
  assertEquals(PF_LRN, mockTransport->sent_messages[0].data[5] & PF_LRN);
  mockTransport->clearMessages();

  // Send NNLRN for another node
   msg = {3, {OPC_NNLRN, 0x01, 0x05}};
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
  assertEquals(0, mockTransport->sent_messages[0].data[5] & PF_LRN);
}

void testEnterLearnModeViaModeInSetup()
{
  test();

  VLCB::Controller controller = createController();
  minimumNodeService->setSetupMode();

  // Learn mode
  VLCB::VlcbMessage msg = {4, {OPC_MODE, 0x01, 0x04, MODE_LEARN_ON}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(1, mockTransport->sent_messages.size());
  assertEquals(OPC_GRSP, mockTransport->sent_messages[0].data[0]);
  assertEquals(OPC_MODE, mockTransport->sent_messages[0].data[3]);
  assertEquals(SERVICE_ID_MNS, mockTransport->sent_messages[0].data[4]); // Yes, filtering is done in MNS.
  assertEquals(CMDERR_INV_CMD, mockTransport->sent_messages[0].data[5]);
}

void testClearEventsNotLearnMode()
{
  test();

  VLCB::Controller controller = createController();
  
  VLCB::VlcbMessage msg = {3, {OPC_NNCLR, 0x01, 0x04}};
  mockTransport->setNextMessage(msg);

  controller.process();

  assertEquals(2, mockTransport->sent_messages.size());
  assertEquals(OPC_CMDERR, mockTransport->sent_messages[0].data[0]);
  assertEquals(CMDERR_NOT_LRN, mockTransport->sent_messages[0].data[3]);
  
  assertEquals(OPC_GRSP, mockTransport->sent_messages[1].data[0]);
  assertEquals(OPC_NNCLR, mockTransport->sent_messages[1].data[3]);
  assertEquals(SERVICE_ID_OLD_TEACH, mockTransport->sent_messages[1].data[4]);
  assertEquals(CMDERR_NOT_LRN, mockTransport->sent_messages[1].data[5]);
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
  testTeachEventIndexedAndClear();
  testEventHashCollisionAndUnlearn(); // tests event lookup in Configuration::findExistingEvent()
  // test error conditions.

// TODO: Error tests
//NNLRN - Done
//NNULN - Done
//NNCLR - Done
//NNEVN - 
//NERD -  (Read all events)
//RQEVN - 
//NENRD -  (Read Event by Index)
//EVULN - 
//REVAL -  (Request EV Read)
//REQEV -  (Read Event Variable in learn mode)
//EVLRN -  (Teach an EV)
//EVLRNI - 

  testEnterLearnModeOldOtherNode();
  testEnterLearnModeViaModeInSetup();
  testClearEventsNotLearnMode();
  testIgnoreMsgsForOtherNodes();
  testIgnoreUnlearnForOtherNodes();
  testIgnoreIfNotInLearnMode();
}
