// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "SerialUserInterface.h"
#include "Controller.h"
#include <Streaming.h>

extern void printConfig();

namespace VLCB
{

void SerialUserInterface::process(const Action *action)
{
  handleAction(action);
  
  processSerialInput();
}

void SerialUserInterface::processSerialInput()
{
  if (Serial.available())
  {
    Configuration *modconfig = controller->getModuleConfig();
    char c = Serial.read();

    switch (c)
    {
      case 'n':
        // node config
        printConfig();

        // node identity
        Serial << F("> VLCB node configuration") << endl;
        Serial << F("> mode = ") << Configuration::modeString(modconfig->currentMode)
               << F(", CANID = ") << modconfig->CANID
               << F(", node number = ") << modconfig->nodeNum << endl;
        Serial << endl;
        break;

      case 'e':
        // EEPROM learned event data table
        Serial << F("> stored events ") << endl;
        Serial << F("  max events = ") << modconfig->getNumEvents()
               << F(" EVs per event = ") << modconfig->getNumEVs() 
               << F(" bytes per event = ") << modconfig->EE_BYTES_PER_EVENT << endl;

        {
          byte uev = 0;
          for (byte j = 0; j < modconfig->getNumEvents(); j++)
          {
            if (modconfig->getEvTableEntry(j) != 0)
            {
              ++uev;
            }
          }

          Serial << F("  stored events = ") << uev << F(", free = ") << (modconfig->getNumEvents() - uev) << endl;
          Serial << F("  using ") << (uev * modconfig->EE_BYTES_PER_EVENT) << F(" of ")
                 << (modconfig->getNumEvents() * modconfig->EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;
        }
        Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

        for (byte j = 0; j < (modconfig->getNumEVs()); j++)
        {
          Serial << _FMT(F("EV% | "), _WIDTHZ(j + 1, 3));
        }

        Serial << F("Hash |") << endl;
        Serial << F(" --------------------------------------------------------------") << endl;

        // for each event data line
        for (byte j = 0; j < modconfig->getNumEvents(); j++)
        {
          if (modconfig->getEvTableEntry(j) != 0)
          {
            Serial << _FMT(F("  %  | "), _WIDTHZ(j, 3));

            // for each data byte of this event
            byte evarray[4];
            modconfig->readEvent(j, evarray);
            for (byte e = 0; e < 4; e++)
            {
              Serial << _FMT(F(" 0x% | "), _WIDTHZ(_HEX(evarray[e]), 2));
            }
            for (byte ev = 1; ev <= modconfig->getNumEVs(); ev++)
            {
              Serial << _FMT(F(" 0x% | "), _WIDTHZ(_HEX(modconfig->getEventEVval(j, ev)), 2));
            }

            Serial << _FMT("%", _WIDTH(modconfig->getEvTableEntry(j), 4)) << endl;
          }
        }

        Serial << endl;

        break;

        // NVs
      case 'v':
        // note NVs number from 1, not 0
        Serial << F("> Node variables") << endl;
        Serial << F("   NV   Val") << endl;
        Serial << F("  --------------------") << endl;

        for (byte j = 1; j <= modconfig->getNumNodeVariables(); j++)
        {
          byte v = modconfig->readNV(j);
          Serial << _FMT(F(" - % : % | 0x%"), _WIDTHZ(j, 2), _WIDTH(v, 3), _HEX(v)) << endl;
        }

        Serial << endl << endl;

        break;

      case 'h':
        // event hash table
        modconfig->printEvHashTable(false);
        break;

      case '*':
        // reboot
        modconfig->reboot();
        break;

      case 'm':
        // free memory
        Serial << F("> free SRAM = ") << modconfig->freeSRAM() << F(" bytes") << endl;
        break;

      case 's': // "s" == "setup"
        //Serial << F("SUI> Requesting mode change") << endl; Serial.flush();
        controller->putAction(ACT_CHANGE_MODE);
        break;

      case '\r':
      case '\n':
        Serial << endl;
        break;

      default:
        Serial << F("> unknown command ") << c << endl;
        break;
    }
  }
}

void SerialUserInterface::handleAction(const Action *action)
{
  if (action == nullptr)
  {
    return;
  }

  switch (action->actionType)
  {
    case ACT_INDICATE_ACTIVITY:
      // Don't indicate this. Too noisy.
      break;

    case ACT_INDICATE_WORK:
      // Don't indicate this. Too noisy.
      break;

    case ACT_INDICATE_MODE:
      indicateMode(action->mode);
      break;

    default:
      break;
  }
}

void SerialUserInterface::indicateMode(VlcbModeParams mode) 
{
  switch (mode) 
  {
    case MODE_NORMAL:
      Serial << "Module in NORMAL mode" << endl;
      break;

    case MODE_UNINITIALISED:
      Serial << "Module in UNINITIALISED mode" << endl;
      break;

    case MODE_SETUP:
      Serial << "Module in SETUP mode" << endl;
      break;

    default:
      break;
  }
}

}