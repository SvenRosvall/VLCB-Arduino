//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Controller.h>                   // Controller class
#include <Switch.h>             // pushbutton switch
#include <LED.h>                // VLCB LEDs
#include <Configuration.h>             // module configuration
#include <Parameters.h>             // VLCB parameters
#include <vlcbdefs.hpp>               // VLCB constants
#include <LEDUserInterface.h>
#include <MinimumNodeServiceWithDiagnostics.h>
#include <CanServiceWithDiagnostics.h>
#include <NodeVariableService.h>
#include <EventConsumerService.h>
#include <EventProducerService.h>
#include <ConsumeOwnEventsService.h>
#include <EventTeachingService.h>
#include <LongMessageService.h>
#include <SerialUserInterface.h>

namespace VLCB
{
void checkStartupAction(byte greenLedPin, byte yellowLedPin, byte pushButtonPin);
void enableDiagnostics();
MinimumNodeService * createMinimumNodeService();
CanService * createCanService(CanTransport *tpt);
NodeVariableService * createNodeVariableService();
ConsumeOwnEventsService * createConsumeOwnEventsService();
EventConsumerService * createEventConsumerService(void (*param)(byte, const VlcbMessage *));
EventTeachingService * createEventTeachingService();
EventProducerService * createEventProducerService();
SerialUserInterface *createSerialUserInterface();
LEDUserInterface * createLEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin);
void setServices(std::initializer_list<Service *> services);
void setVersion(char maj, char min, char beta);
void setModuleId(byte manu, byte moduleId);
void setName(char *mname);
void setNumNodeVariables(byte n);
void setMaxEvents(byte n);
void setNumProducedEvents(byte n);
void setNumEventVariables(byte n);

VlcbModeParams getCurrentMode();
byte getCANID();
unsigned int getNodeNum();
byte readNV(byte nv);
byte getEventEVval(byte idx, byte evnum);
void resetModule();

void begin();
void process();
void sendEvent(bool state, byte channel);
}