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

SerialUserInterface::SerialUserInterface(Configuration * modconfig, Transport *transport)
  : modconfig(modconfig)
  , transport(transport)
{
}

void SerialUserInterface::run()
{
  byte uev = 0;
  char msgstr[32], dstr[32];

  if (Serial.available())
  {
    char c = Serial.read();

    switch (c)
    {
      case 'n':
        // node config
        printConfig();

        // node identity
        Serial << F("> VLCB node configuration") << endl;
        Serial << F("> mode = ") << (modconfig->currentMode == MODE_NORMAL ? "Normal" : "Unitialised") << F(", CANID = ") << modconfig->CANID << F(", node number = ") << modconfig->nodeNum << endl;
        Serial << endl;
        break;

      case 'e':
        // EEPROM learned event data table
        Serial << F("> stored events ") << endl;
        Serial << F("  max events = ") << modconfig->EE_MAX_EVENTS << F(" EVs per event = ") << modconfig->EE_NUM_EVS << F(" bytes per event = ") << modconfig->EE_BYTES_PER_EVENT << endl;

        for (byte j = 0; j < modconfig->EE_MAX_EVENTS; j++)
        {
          if (modconfig->getEvTableEntry(j) != 0)
          {
            ++uev;
          }
        }

        Serial << F("  stored events = ") << uev << F(", free = ") << (modconfig->EE_MAX_EVENTS - uev) << endl;
        Serial << F("  using ") << (uev * modconfig->EE_BYTES_PER_EVENT) << F(" of ") << (modconfig->EE_MAX_EVENTS * modconfig->EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;

        Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

        for (byte j = 0; j < (modconfig->EE_NUM_EVS); j++)
        {
          sprintf(dstr, "EV%03d | ", j + 1);
          Serial << dstr;
        }

        Serial << F("Hash |") << endl;
        Serial << F(" --------------------------------------------------------------") << endl;

        // for each event data line
        for (byte j = 0; j < modconfig->EE_MAX_EVENTS; j++)
        {
          if (modconfig->getEvTableEntry(j) != 0)
          {
            sprintf(dstr, "  %03d  | ", j);
            Serial << dstr;

            // for each data byte of this event
            byte evarray[4];
            modconfig->readEvent(j, evarray);
            for (byte e = 0; e < 4; e++)
            {
              sprintf(dstr, " 0x%02hx | ", evarray[e]);
              Serial << dstr;
            }
            for (byte ev = 1; ev <= modconfig->EE_NUM_EVS; ev++)
            {
              sprintf(dstr, " 0x%02hx | ", modconfig->getEventEVval(j, ev));
              Serial << dstr;
            }

            sprintf(dstr, "%4d |", modconfig->getEvTableEntry(j));
            Serial << dstr << endl;
          }
        }

        Serial << endl;

        break;

        // NVs
      case 'v':
        // note NVs number from 1, not 0
        Serial << "> Node variables" << endl;
        Serial << F("   NV   Val") << endl;
        Serial << F("  --------------------") << endl;

        for (byte j = 1; j <= modconfig->EE_NUM_NVS; j++)
        {
          byte v = modconfig->readNV(j);
          sprintf(msgstr, " - %02d : %3hd | 0x%02hx", j, v, v);
          Serial << msgstr << endl;
        }

        Serial << endl << endl;

        break;

        // CAN bus status
      case 'c':
        Serial << F(" messages received = ") << transport->receiveCounter()
               << F(", sent = ") << transport->transmitCounter()
               << F(", receive errors = ") << transport->receiveErrorCounter()
               << F(", transmit errors = ") << transport->transmitErrorCounter()
               << F(", error status = ") << transport->errorStatus()
        << endl;
        break;

      case 'h':
        // event hash table
        modconfig->printEvHashTable(false);
        break;

      case 'y':
        // reset CAN bus and VLCB message processing
        transport->reset();
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
        requestedAction = CHANGE_MODE;
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

void SerialUserInterface::indicateResetting()
{
  Serial << "Resetting module." << endl;
}

void SerialUserInterface::indicateResetDone()
{
  Serial << "Module reset done" << endl;

}

void SerialUserInterface::indicateActivity()
{
}

bool SerialUserInterface::resetRequested()
{
  return isResetRequested;
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

UserInterface::RequestedAction SerialUserInterface::checkRequestedAction()
{
  UserInterface::RequestedAction oldRequestedAction = requestedAction;
  requestedAction = NONE;
  return oldRequestedAction;
}

}