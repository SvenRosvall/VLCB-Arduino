//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <memory>
#include "MockUserInterface.h"
#include "MockTransport.h"
#include "Service.h"

const int MODULE_ID = 253;
const unsigned char moduleName[] = {'t', 'e', 's', 't', 'i', 'n', 'g', '\0'};

extern std::unique_ptr<MockUserInterface> mockUserInterface;
extern std::unique_ptr<MockTransport> mockTransport;
extern std::unique_ptr<VLCB::Configuration> configuration;

// Create a Configuration object.
VLCB::Configuration * createConfiguration();
VLCB::Configuration * createConfiguration(VLCB::Storage * mockStorage);
// Use MockTransport to mock out the whole transport part.
VLCB::Controller createController(std::initializer_list<VLCB::Service *> services);
// Use a provided transport.
VLCB::Controller createController(VLCB::Transport * trp, std::initializer_list<VLCB::Service *> services);
