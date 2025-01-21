//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Service.h>

namespace VLCB
{
class CanTransport;
class MinimumNodeService;
class CanService;

class ServiceFactory
{
public:
  virtual MinimumNodeService * createMinimumNodeService() = 0;
  virtual CanService * createCanService(CanTransport *tpt) = 0;
};

class ServiceFactoryNoDiagnostics : public ServiceFactory
{
public:
  virtual MinimumNodeService * createMinimumNodeService() override;
  virtual CanService * createCanService(CanTransport *tpt) override;
};

class ServiceFactoryWithDiagnostics : public ServiceFactory
{
public:
  virtual MinimumNodeService * createMinimumNodeService() override;
  virtual CanService * createCanService(CanTransport *tpt) override;
};

}