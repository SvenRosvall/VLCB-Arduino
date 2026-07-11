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

This example sketch uses serial communication using the GridConnect protocol.

## Using VLCB4in4out

VLCB4in4out uses the following pins for VLCB Initialisation:

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
A value of 255 identifies the event as a request for [Start-of-Day reporting](#start-of-day-event),
i.e. send an event for each input to update control panels etc, of the current
state.
A value of 1 to 4 will identify the source (in this case switch) for a produced
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

### Start-of-Day event
When a Start-of-Day event is received the node will respond
with an event for each input (switch) to indicate its state.

Note that ON-only switches and OFF-only switches do not
generate these state events as the state will always be ON or OFF.

A toggle switch should ideally remember its state when
powering off the node. 
Instead, the reported state for a toggle switch will be the
default state unless it has not changed state since start up 
of the node.

Instead of sending all response events in a loop a [TimedResponse](../html.sketch/_timed_response_8h.html) instance, StartOfDayResponse, is used.
This object is called by the library at an interval to separate the responses
in time.
Each call will use a different sequence number which is used to identify which
switch to report state for.

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


 
 
 
 
