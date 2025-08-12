//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "VlcbCommon.h"
#include "MockStorage.h"
#include "Parameters.h"
#include "Controller.h"

std::unique_ptr<MockUserInterface> mockUserInterface;
std::unique_ptr<VLCB::Configuration> configuration;

// For use by tests of Configuration
VLCB::Configuration * createConfiguration()
{
  static std::unique_ptr<MockStorage> mockStorage;
  mockStorage.reset(new MockStorage);
  VLCB::Configuration *configuration = createConfiguration(mockStorage.get());
  configuration->EE_NVS_START = 10;
  configuration->EE_NUM_NVS = 4;
  configuration->EE_EVENTS_START = 20;
  configuration->EE_MAX_EVENTS = 20;
  configuration->EE_PRODUCED_EVENTS = 1;
  configuration->EE_NUM_EVS = 2;
  configuration->setFcuCompatability(false);
  configuration->begin();
  return configuration;
}

// For use with createController()
VLCB::Configuration * createConfiguration(VLCB::Storage * mockStorage)
{
  auto configuration = new VLCB::Configuration(mockStorage);
  return configuration;
}

VLCB::Controller createController(const std::initializer_list<VLCB::Service *> services)
{
  return createController(MODE_NORMAL, services);
}

VLCB::Controller createController(VlcbModeParams startupMode, const std::initializer_list<VLCB::Service *> services)
{
  // Use pointers to objects to create the controller with.
  // Use unique_ptr so that next invocation deletes the previous objects.

  configuration.reset(createConfiguration());

  VLCB::Controller controller(configuration.get(), services);
  if (startupMode == MODE_NORMAL)
  {
    configuration->setModuleNormalMode(0x0104);
  }
  static std::unique_ptr<VLCB::Parameters> params;
  params.reset(new VLCB::Parameters(*configuration));
  params->setVersion(1, 1, 'a');
  params->setModuleId(MODULE_ID);

  // assign to Controller
  controller.setParams(params->getParams());
  controller.setName(moduleName);
  return controller;
}

void process(VLCB::Controller &controller)
{
  const int MAX_PROCESS_COUNT = 30;
  controller.process();
  for (int i = 0 ; controller.pendingAction() && i < MAX_PROCESS_COUNT ; ++i)
  {
    controller.process();
  }
}
