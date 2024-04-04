# VLCB_1in1out_SimpleCLI_MEGA

This code demonstrates the use of the VLCB SimpleCLIUserInterface as an alternative to the SerialUserInterface.

This is made possible by the use of the third party library SimpleCLI to define commands and a help system.

This code is too large to run on an Arduino UNO or NANO and has been set to compile for a MEGA 1280 or MEGA 2560.

## SimpleCLI

This library enables commands with optional parameters on the Serial interface.

https://github.com/SpacehuhnTech/SimpleCLI

It is possible for the commands to execute a callback function within the user code.

## Motivation

The origins of the SerialUserInterface for VLCB lie in the development of a serial interface for Arduino CBUS applications. The idea of this is to allow the user to configure the module using the serial interface of the Arduino IDE. In the CBUS version the code to process the input is included in the application.

The VLCB SerialUserInterface has been developed to provide a similar interface for the VLCB-Arduino library. This is be provided as a separate service.

SimpleCLIUserInterface is an alternative to the SerialUserInterface which allows for much more flexibility in what can be input using the serial interface.

With the SerialUserInterface the commands are single characters.

SimpleCLIUserInterface allow for word commands which can have parameters in various combinations. The commands can also run callback functions and this has been used to implement the single letter commands available in the SerialUserInterface.

Several different commands can be combined together e.g. "a,b,c" will respond to "a" or "b" or "c".

## Example

This example shows how the interface can be use to construct a help system for VLCB examples.

It is also possible for the user to add commands to the help system.

The available help commands are: help, helpAbout.

There is also a command which provides the single character commands for the VLCB interface:

n,e,v,c,h,y,s,z,*

These are the same as in the SerialUserInterface except for an added "z".

There are two examples of user added commands:

some/thing  - this will respond to "some" or "something".

a,b  - this will respond to "a" or "b"

## Assistance and help

For assistance with this code please contact John Fletcher <M6777> john@bunbury28.plus.com
