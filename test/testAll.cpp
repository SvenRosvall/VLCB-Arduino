//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <map>
#include <string>
#include <iostream>
#include "testArduino.hpp"
#include "TestTools.hpp"

void testMinimumNodeService();
void testNodeVariableService();
void testCanService();
void testEventProducerService();
void testEventConsumerService();
void testEventTeachingService();
void testConsumeOwnEventsService();
void testLongMessageService();

// Remaining services to implement
//Bootloader (the CBUS PIC version) service #10
//Event Acknowledge Service #9

std::map<std::string, void (*)()> suites = {
        {"Arduino", testArduino},
        {"MinimumNodeService", testMinimumNodeService},
        {"NodeVariableService", testNodeVariableService},
        {"CanService", testCanService},
        {"EventProducerService", testEventProducerService},
        {"EventConsumerService", testEventConsumerService},
        {"EventTeachingService", testEventTeachingService},
        {"ConsumeOwnEventsService", testConsumeOwnEventsService},
        {"LongMessageService", testLongMessageService}
};

int main(int argc, const char * const * argv)
{
  int totalFailures = 0;
  if (*++argv == nullptr)
  {
    for (auto const &i : suites)
    {
      suite(i.first);
      i.second();
      totalFailures += failures(); 
    }
  }
  else
  {
    bool needHelp = false;
    while (const char * arg = *argv++)
    {
      auto found = suites.find(arg);
      if (found != suites.end())
      {
        found->second();
        totalFailures += failures();
      }
      else
      {
        std::cout << "Cannot find test suite '" << arg << "'" << std::endl;
        needHelp = true;
      }
    }
    if (needHelp)
    {
      std::cout << "The following test suites are available:" << std::endl;
      for (auto & suite : suites)
      {
        std::cout << "  " << suite.first << std::endl;
      }
    }
  }
  if (totalFailures > 0)
  {
    std::cout << "Completed with totalFailures. " << totalFailures << " test(s) failed." << std::endl;
  }
  return totalFailures;
}
