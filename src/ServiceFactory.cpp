//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "ServiceFactory.h"
#include "CanService.h"
#include "MinimumNodeServiceWithDiagnostics.h"
#include "CanServiceWithDiagnostics.h"
#include "SerialUserInterface.h"
#include "LEDUserInterface.h"

namespace VLCB
{

MinimumNodeService *ServiceFactoryNoDiagnostics::createMinimumNodeService()
{
  return new MinimumNodeService();
}

MinimumNodeService *ServiceFactoryWithDiagnostics::createMinimumNodeService()
{
  return new MinimumNodeServiceWithDiagnostics();
}

CanService *ServiceFactoryNoDiagnostics::createCanService(CanTransport *tpt)
{
  return new CanService(tpt);
}

CanService *ServiceFactoryWithDiagnostics::createCanService(CanTransport *tpt)
{
  return new CanServiceWithDiagnostics(tpt);
}

SerialUserInterface *ServiceFactory::createSerialUserInterface()
{
  return new SerialUserInterface();
}

LEDUserInterface * ServiceFactory::createLEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
{
  return new LEDUserInterface(greenLedPin, yellowLedPin, pushButtonPin);
}

}