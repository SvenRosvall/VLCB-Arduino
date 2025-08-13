//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "VLCB.h"
#include "ServiceFactory.h"

namespace VLCB
{
ServiceFactory * svcFactory = new ServiceFactoryNoDiagnostics;

Configuration modconfig;               // configuration object
Controller controller(&modconfig); // Controller object

Parameters params(modconfig);

void checkStartupAction(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
{
  // Called at the startup of the module. Check if the push button is pressed
  // at startup and check for a sequence of presses for actions.
  // TODO: See how this is described in VLCB Tech Intro doc.
  // Might return a value for actions the application may take such as enter test mode.

  // For now, we just do a factory reset if switch is depressed at startup
  if (digitalRead(pushButtonPin))
  {
    VLCB::resetModule();
  }
}

void enableDiagnostics()
{
  delete svcFactory;
  svcFactory = new ServiceFactoryWithDiagnostics;
}

MinimumNodeService * createMinimumNodeService()
{
  return svcFactory->createMinimumNodeService();
}

CanService * createCanService(CanTransport *tpt)
{
  return svcFactory->createCanService(tpt);
}

SerialUserInterface *createSerialUserInterface()
{
  return svcFactory->createSerialUserInterface();
}

LEDUserInterface * createLEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
{
  return svcFactory->createLEDUserInterface(greenLedPin, yellowLedPin, pushButtonPin);
}

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