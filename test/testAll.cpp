//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <map>
#include <string>
#include <iostream>
#include "testArduino.hpp"

void testMinimumNodeService();
void testNodeVariableService();
void testCanService();
void testEventProducerService();

// Remaining services to test:
//Old (CBUS mechanism) Event Teaching Service #4
//Event Consumer Service #5
//Stream service #17
//Consume Own Events Service #8

// Remaining services to implement
//Bootloader (the CBUS PIC version) service #10
//Event Acknowledge Service #9

std::map<std::string, void (*)()> suites = {
        {"Arduino", testArduino},
        {"MinimumNodeService", testMinimumNodeService},
        {"NodeVariableService", testNodeVariableService},
        {"CanService", testCanService},
};

int main(int argc, const char * const * argv)
{
  if (*++argv == nullptr)
  {
    for (auto const &i : suites)
    {
      i.second();
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
  return 0;
}
