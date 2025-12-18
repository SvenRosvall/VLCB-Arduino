# VLCB4IN4OUT

An Arduino program to allocate 4 switches as inputs and 4 outputs to LEDs.

Key Features:
- MERG VLCB interface.
- LED flash rate selectable from event variables.
- Switch function controllable by node variables.
- Modular construction to ease adaptation to your application.

## Overview

The program is written in C++ but you do not need to understand this to use the program.
The program includes a library that manages the LED functionality.

NOTE: It can get difficult when using DEBUG to know where the message has come from. Those
emanating from the sketch are preceded with the letters sk for sketch. 

## Using VLCB4in4out

The MCP2515 interface requires five Arduino pins to be allocated. Three of these are fixed
in the architecture of the Arduino processor. One pin must be connected to an interrupt
capable Arduino pin. You can only use pin 2 or 3 on an Uno. The Chip Select pin can be 
freely defined. This example is configured for use with an Uno but can easily be adapted
for use with other Arduino types.

If the MERG Kit 110 CAN Shield is used, the following pins are connected by default:

Pin | Description
--- | ---
Digital pin 2 | Interrupt CAN
Digital pin 10| (SS)    CS    CAN
Digital pin 11| (MOSI)  SI    CAN
Digital pin 12| (MISO)  SO    CAN
Digital pin 13| (SCK)   Sck   CAN

Using the CAN Shield, the following pins are used for VLCB Initialisation:

Pin | Description
--- | ---
Digital pin 4 | VLCB Green LED
Digital pin 7 | VLCB Yellow LED
Digital pin 8 | VLCB Switch

**It is the users responsibility that the total current that the Arduino is asked to supply 
stays within the capacity of the on-board regulator.  Failure to do this will result in 
terminal damage to your Arduino.**

Pins defined as inputs are active low.  That is to say that they are pulled up by an 
internal resistor. The input switch should connect the pin to 0 Volts.

Pins defined as outputs are active high.  They will source current to (say) an LED. It is 
important that a suitable current limiting resistor is fitted between the pin and the LED 
anode.  The LED cathode should be connected to ground.

### Library Dependencies

As well as the VLCB Library suite associated with this example, the following third party
libraries are required:

Library | Purpose
---------------|-----------------
Streaming.h  |*C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)*
ACAN2515.h   |*library to support the MCP2515/25625 CAN controller IC*

### Application Configuration

The example is configured for use with the MERG Kit 110 CAN Shield. This has a crystal
frequency of 16MHz.  If you use another CAN2515 board, the crystal frequency may be 8MHz.
In this case it will be necessary to change the code at or about line 173 appropriately:
```
can2515.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module
```

### VLCB Op Codes

The following Op codes are supported:

OP_CODE | HEX | Function
----------|---------|---------
 OPC_ACON | 0x90 | On event
 OPC_ACOF | 0x91 | Off event
 OPC_ASON | 0x98 | Short event on
 OPC_ASOF | 0x99 | Short event off

## Events

### Event Variables

Event Variable 1 (EV1) identifies a consumed only event if its value is zero.
A non-zero value will identify the source (in this case switch) for a produced
event.

### Consumed Events

Event Variables control the action to take when a consumed event is received.
The number of Event Variables (EV) is equal to the number of LEDs plus one.

Event Variable 2 (EV2) controls the first LED pin in the ```LED``` array. 
EV3 controls the second LED pin, etc.

The LEDs are controlled by the LEDControl class.  This allows for any LED to be
switched on, switched off or flashed at a rate determined in the call:
LEDControl::flash(unsigned int period)

The following EV values are defined to control the LEDs in this example:

 EV Value | Function
--------|-----------
 0 | LED off
 1 | LED on
 2 | LED flash at 500mS
 3 | LED flash at 250mS

### Produced Events

The Events Table in an Uninitialised module is empty. When the module changes to
Normal Mode from Uninitialised, the Events Table is populated with a default
producer event for each of the four switches (or, in general, each producer item).
These default events can be removed by using OPC_EVULN whilst the module is in
learn mode.

A module can also be taught a short event or, indeed, a spoof event. This is
easily done using any of the normal FCU teach techniques.  For example, a short
event can be dragged from the Software Node and dropped onto the VLCB4in4out in
the Node Window.  When the event variable dialogue opens, put the index number of
the switch to be associated with the event in EV1 and press OK.  The selected
switch will now generate that short event (but see [FCU Anomalies](#fcu-anomalies) below).

If a switch is operated, and its associated NV value is set to an action,
but there is no event with EV1 value matching the switch number, then a
default event will be created for that switch.
This event will have a node number matching the node number of this node and
an event number that matches the switch number, if available, otherwise the 
lowest available event number will be used.

### Consume Own Events

If the Produced Events Service and the Consume Events Service are both applied,
the Consume Own Events Service can also be enabled.  This service provides a 
buffer that will pass the produced event back to the Consumed Event Service.
A Consume Own Event still only has one entry in the Event Table.  If the Event
Variables 2 onwards are left as 0 or any undefined value, then the
Consume Events Service will do nothing.  Note that an EV will have a default
value of 0xFF if not written to. Event Variable 1 will contain the value of
the producer trigger or switch. If the Event Variables are populated as shown
in the table in the Consumed Events Section above, the LEDs will respond
accordingly to a Produced Event.

## Node Variables

Node Variables control the action to take when a switch is pressed or depressed.

The number of Node Variables (NV) is equal to the number of switches.
NV1 corresponds to the first pin defined in the array ```SWITCH```, 
NV2 corresponds to the second pin in that array, etc.

The following NV values define input switch function:

NV Value | Function
--------|--------
 0 | Do nothing
 1 | On/Off switch
 2 | On only push button
 3 | Off only push button
 4 | On/Off toggle push button
 
## Set Up

The module is initialised by pressing the VLCB button (formerly CBUS button) for
6 seconds when the green LED goes off and the Yellow LED flashes. A module name and
Node Number can then be set via the FCU in the normal manner.

## FCU Anomalies

Whilst the FCU will show newly taught events, if these re-assign a switch, as when
teaching a short event, it will not, for some reason, remove the now gone original
event automatically. It is necessary to highlight the redundant event and use
alt-D to remove it.

It should be noted that the use of alt-D only removes an event from the FCU internal
table.  It does not remove the event from the Arduino events table.


 
 
 
 
