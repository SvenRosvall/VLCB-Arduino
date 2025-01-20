//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "VLCB.h"

namespace VLCB
{
Configuration modconfig;               // configuration object
Controller controller(&modconfig); // Controller object
//MinimumNodeServiceWithDiagnostics mnService;

VLCB::Parameters params(modconfig);

void setServices(std::initializer_list<Service *> services)
{
  controller.setServices(services);
}

void setName(char *mname)
{
  controller.setName(mname);
}

void setVersion(char maj, char min, char beta)
{
  params.setVersion(maj, min, beta);
}

void setModuleId(byte manu, byte moduleId)
{
  params.setManufacturer(manu);
  params.setModuleId(moduleId);
}

void setNumNodeVariables(byte n)
{
  modconfig.EE_NUM_NVS = n;
}

void setMaxEvents(byte n)
{
  modconfig.EE_MAX_EVENTS = n;
}

void setNumProducedEvents(byte n)
{
  modconfig.EE_PRODUCED_EVENTS = n;
}

void setNumEventVariables(byte n)
{
  modconfig.EE_NUM_EVS = n;
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

byte getEventEVval(byte idx, byte evnum)
{
  return modconfig.getEventEVval(idx, evnum);
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