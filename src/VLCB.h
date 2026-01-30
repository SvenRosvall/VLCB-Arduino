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
void setServices(std::initializer_list<Service *> services);
void setVersion(char maj, char min, char beta);
void setModuleId(byte manu, byte moduleId);
void setName(char *mname);
void setNumNodeVariables(byte n);
void setEventsStart(byte n);
void setMaxEvents(byte n);
void setNumEventVariables(byte n);

VlcbModeParams getCurrentMode();
byte getCANID();
unsigned int getNodeNum();
unsigned int getFreeEEPROMbase();
byte readNV(byte nv);
void writeNV(byte nv, byte val);
byte getEventEVval(byte idx, byte evnum);
byte findExistingEventByEv(int evIndex, byte value);
byte findExistingEvent(unsigned int nn, unsigned int en);
bool isEventIndexValid(byte eventIndex);
bool doesEventExistAtIndex(byte eventIndex);
byte findEmptyEventSpace();
void createEventAtIndex(byte eventIndex, unsigned int nn, unsigned int en);
void writeEventVariable(byte eventIndex, byte evIndex, byte value);

bool sendMessageWithNN(VlcbOpCodes opc);
bool sendMessageWithNN(VlcbOpCodes opc, byte b1);
bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2);
bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3);
bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4);
bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4, byte b5);

void resetModule();

void begin();
void process();
}