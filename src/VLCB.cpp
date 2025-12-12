//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "VLCB.h"

namespace VLCB
{
namespace
{
  Configuration modconfig;               // configuration object
  Controller controller(&modconfig); // Controller object
}

void checkStartupAction(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
{
  // Called at the startup of the module. Check if the push button is pressed
  // at startup and check for a sequence of presses for actions.
  // TODO: See how this is described in VLCB Tech Intro doc.
  // Might return a value for actions the application may take such as enter test mode.

  // For now, we just do a factory reset if switch is depressed at startup
  pinMode(pushButtonPin, INPUT_PULLUP);
  if (digitalRead(pushButtonPin) == LOW)
  {
    VLCB::resetModule();
  }
}

void setServices(std::initializer_list<Service *> services)
{
  controller.setServices(services);
}

void setName(char *mname)
{
  modconfig.setName(mname);
}

void setVersion(char maj, char min, char beta)
{
  modconfig.setVersion(maj, min, beta);
}

void setModuleId(byte manu, byte moduleId)
{
  modconfig.setModuleId(manu, moduleId);
}

void setNumNodeVariables(byte n)
{
  modconfig.setNumNodeVariables(n);
}

void setMaxEvents(byte n)
{
  modconfig.setNumEvents(n);
}

void setEventsStart(byte n)
{
  modconfig.EE_EVENTS_START = n;
}

void setNumEventVariables(byte n)
{
  modconfig.setNumEVs(n);
}

VlcbModeParams getCurrentMode()
{
  return modconfig.currentMode;
}

byte getCANID()
{
  return modconfig.CANID;
}

unsigned int getNodeNum()
{
  return modconfig.nodeNum;
}

byte readNV(byte nv)
{
  return modconfig.readNV(nv);
}

void writeNV(byte nv, byte val)
{
  modconfig.writeNV(nv, val);
}

byte getEventEVval(byte idx, byte evnum)
{
  return modconfig.getEventEVval(idx, evnum);
}

byte findExistingEventByEv(int evIndex, byte value)
{
  return modconfig.findExistingEventByEv(evIndex, value);
}

byte findExistingEvent(unsigned int nn, unsigned int en)
{
  return modconfig.findExistingEvent(nn, en);
}

bool isEventIndexValid(byte eventIndex)
{
  return eventIndex < modconfig.getNumEvents();
}

bool doesEventExistAtIndex(byte eventIndex)
{
  return isEventIndexValid(eventIndex) && modconfig.isEventSlotInUse(eventIndex);
}

byte findEmptyEventSpace()
{
  return modconfig.findEventSpace();
}

void createEventAtIndex(byte eventIndex, unsigned int nn, unsigned int en)
{
  modconfig.writeEvent(eventIndex, nn, en);
  modconfig.updateEvHashEntry(eventIndex);
  for (byte i = 1; i <= modconfig.getNumEVs(); i++)
  {
    modconfig.writeEventEV(eventIndex, i, 0);
  }
}

void writeEventVariable(byte eventIndex, byte evIndex, byte value)
{
  modconfig.writeEventEV(eventIndex, evIndex, value);
}

unsigned int getFreeEEPROMbase()
{
  return modconfig.EE_FREE_BASE;
}

void resetModule()
{
  modconfig.resetModule();
}

void begin()
{
  controller.updateParamFlags();
  controller.begin();
}

void process()
{
  controller.process();
}

}