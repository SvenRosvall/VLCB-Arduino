// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "SimpleCLIUserInterface.h"
#include "Controller.h"
#include <Streaming.h>
#include <SimpleCLI.h>

extern void printConfig();


namespace VLCB
{

// Create CLI Object
SimpleCLI simpleCli(CLI_COMMAND_QUEUE_SIZE); // Default of 10 commands

Command cmdHelp;
Command cmdHelpWithArg;
Command cmdSerialInterface;

//////////////////////////////////////////////////////////////////
// See Modern C++ Design, Andrei Alexandrescu, 2001(!) page 116
// First declare a typedef for a pointer to what is needed.
// Note the argument type.
typedef void (SimpleCLIUserInterface::* TpMemFun)(cmd *);
// Then create an object which implements tha callback
TpMemFun pSerialCallback = &SimpleCLIUserInterface::serialCallback;
// An instance of class SimpleUserInterace is also needed.
// Where is it? Declared in the top level code.
// This is the actual call to use the callback
//     (simpleCLIUserInterface.*pSerialCallback)(c);
// It is used in local_templateCallback(cmd *c) - see below.
/////////////////////////////////////////////////////////////



SimpleCLIUserInterface::SimpleCLIUserInterface(Transport *transport)
  : transport(transport)
{
}

void SimpleCLIUserInterface::setController(Controller *ctrl)
{
  this->controller = ctrl;
  this->modconfig = ctrl->getModuleConfig();
}

void SimpleCLIUserInterface::process(const Action *action)
{
  handleAction(action);
  
  //This replaces processSerialInput(); 
  getHelp();
}

// This will hold a local copy of the first char of the input.
char local_serialChar = '0'; 

void SimpleCLIUserInterface::serialCallback(cmd *c) {
    Command cmd(c); // Create wrapper object
    Serial.print("called as ");
	// Copy the char into the instance.
	serialChar = local_serialChar;
    Serial.print(serialChar);
    Serial.print(" : ");
    Serial.println(cmd.toString());
	processSerialInputImpl(serialChar); 
}

void local_serialCallback(cmd *c) {
    Command cmd(c); // Create wrapper object
    Serial.print("called as ");
    Serial.print(local_serialChar);
    Serial.print(" : ");
    Serial.println(cmd.toString());
	(simpleCLIUserInterface.*pSerialCallback)(c);
}

// This is called from setupVLCB().
void SimpleCLIUserInterface::setupHelp()
{
    Serial << "> VLCB help system setting up" << endl;

    cmdHelp = simpleCli.addCommand("help");
    cmdHelp.setDescription(" Get help!");

    cmdHelpWithArg = simpleCli.addCommand("helpAbout");
    cmdHelpWithArg.addArg("topic","help");
    cmdHelpWithArg.setDescription(" Get help on a topic!");

    cmdSerialInterface = simpleCli.addCommand("n,e,v,c,h,y,s,z,*",local_serialCallback);
	cmdSerialInterface.setDescription("serial commands n, e, v, c, h, y, s, z and * for the module");
	
    Serial.println("> VLCB help system started!!");
}

void SimpleCLIUserInterface::getHelp()
{
   if (Serial.available()) {
        String input = Serial.readStringUntil('\n');

        if (input.length() > 0) {
            Serial.print("# ");
            Serial.println(input);
            serialChar = input[0];
            local_serialChar = serialChar;			
            simpleCli.parse(input);
        }
    }

    if (simpleCli.available()) {
        Command c = simpleCli.getCmd();

        int argNum = c.countArgs();

        Serial.print("> ");
        Serial.print(c.getName());
        Serial.print(' ');

        for (int i = 0; i<argNum; ++i) {
            Argument arg = c.getArgument(i);
            // if(arg.isSet()) {
            Serial.print(arg.toString());
            Serial.print(' ');
            // }
        }
		
		Serial.println();

        if (c == cmdHelp) {
            Serial.println("Help:");
            Serial.println(simpleCli.toString());
        } else if (c == cmdHelpWithArg) {
            Serial.print("HelpAbout:");
            Argument arg = c.getArgument(0);
            Serial.println(arg.getValue());
            Command cc = simpleCli.getCmd(arg.getValue());
            if (cc.hasDescription()) {
              Serial.println(cc.toString());
            } else {
              Serial.print("No description available for ");
              Serial.println(arg.getValue());
            }
        }
	}
}

void SimpleCLIUserInterface::processSerialInput()
{

  if (Serial.available())
  {
     char c = Serial.read();
	 
	 processSerialInputImpl(c);

  }
}

void SimpleCLIUserInterface::processSerialInputImpl(char c)
{
  byte uev = 0;
  char msgstr[32], dstr[32];

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

      case 'z':
        // reboot
        modconfig->resetModule();
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

void SimpleCLIUserInterface::handleAction(const Action *action)
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

    case ACT_INDICATE_MODE:
      indicateMode(action->mode);
      break;

    default:
      break;
  }
}

void SimpleCLIUserInterface::indicateMode(VlcbModeParams mode) 
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