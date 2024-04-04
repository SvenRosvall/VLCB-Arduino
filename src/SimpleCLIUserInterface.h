// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include "Configuration.h"
#include "Transport.h"
#include <SimpleCLI.h>

#define CLI_COMMAND_QUEUE_SIZE 10 

namespace VLCB
{

class SimpleCLIUserInterface : public Service
{
public:
  SimpleCLIUserInterface(Transport *transport);
  virtual void setController(Controller *ctrl) override;
  virtual byte getServiceID() override { return 0; };
  virtual byte getServiceVersionID() override { return 1; };

  virtual void process(const Action *action) override;
  
private:
  Controller * controller;
  Configuration * modconfig;
  Transport * transport;
  bool isResetRequested = false;
  char serialChar = '0';
  
  
  void handleAction(const Action *action);
  void processSerialInput();
  void processSerialInputImpl(char c);
  void indicateMode(VlcbModeParams i);
  void getHelp();
public:
  void serialCallback(cmd *c);
  void setupHelp();
  char getFirstChar() const {return serialChar; }
  
};

extern SimpleCLI simpleCli;

} // end namespace VLCB

// Allow access to the instance in SimpleCLIUserInterface.cpp
// Note that this is not in the VLCB namespace.
extern VLCB::SimpleCLIUserInterface simpleCLIUserInterface;



