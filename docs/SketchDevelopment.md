# Developing a sketch for a VLCB module

The VLCB-Arduino library is created to make it easy to develop a VLCB module
on the Arduino platform.

You need to be familiar with CBUS/VLCB concepts such as _nodes_, _events_,
_node variables_, and _event variables_.

There are a few steps involved when developing a VLCB module.
These steps are described below.

## Decide on Node Variable and Event Variable structure
It is important to identify what node variables and event variables
will be needed by the module as it is difficult to change these in later 
versions of the module code.

Node variables are used to define module behaviours such as timing and threshold
parameters.
There can be node variables that define behaviour of individual I/O pins.

Event variables are used to define what to do when a consumed event is
received. 
Event variables are also used for produced events to control how and when
these events are produced.

There are two strategies for organising events and event variables: 1) classic
events, and 2) slot events.
These strategies deal with how events are stored in the events table and 
what event variables are required.

### Classic Events
When using classic events the events are stored at any location in the events
table.
The behaviours for consumed and produced events are controlled by the values
of event variables. 

The example sketch "4in4out" uses classic events.
Its event variable strategy is described below.

Produced events are sent out when an input pin changes state.
One event variable is deciding which input pin causes this event to be produced.

Consumed events are used to sent output pins such as LEDs. 
There is one event variable for each LED. 
The value of the event variable indicates if the LED shall be turned on or blink.
Multiple event variables can be set so that more than one LED is turned on
for the same consumed event.

A produced event can also be "self consumed" if it also has the event variables
set for controlling LEDs.

### Slot Events
For slot events the position (slot) in the event table defines the behaviour
of the events.

The example sketch "4in4out_slot" uses slot events. Its event variable strategy
is described below.

Each input pin has a dedicated slot in the event table. 
When an input pin changes that slot is used to send the event in that slot.

Consumed events are looked up in the event table. 
The slots they are found in defines which LED shall be changed.
The same event can be stored in multiple slots to allow multiple LEDs to
be changed.
There is only one event variable here to say if the LED shall be turned on or blink.

A produced event can be "self consumed" if the same event appears in a
slot for changing an LED.

The slot event strategy reqires a larger event table but less event variables.
Total memory consumption for the events table is very similar to classic
events strategy.

## Choosing a module identifier
A module identifier consists of a _Manufacturer ID_ and a _Module ID_.
The combination of these two must be unique.

VLCB modules that are generally available, through MERG kitlocker or
shared between members, have a _Manufacturer ID_ set to 165 (MANU_MERG) and
the _Module ID_ is defined in [CBUSDEFS](https://github.com/MERG-DEV/cbusdefs)
which is maintained by Pete Brownlow.

Modules that are being developed or only used by a single person do not
have to be defined in CBUSDEFS. 
Instead, use the _Manufacturer ID_ 13 (MANU_DEV).
This _Manufacturer ID_ is intended for local development and the 
_Module ID_ only need to be unique on this local layout bus. 

The example sketches use _Manufacturer ID_ 13 (MANU_DEV).
If you use any of these sketches as a base for your own developed module 
you must change the _Module ID_ to something that is unique in your
layout bus.

## Programming the VLCB module

The VLCB module code can be written in two different styles.
One style is to use the objects described below and calling member functions of these objects.
This give you great flexibility and control but makes things a bit complicted.

The other style is to use functions in the VLCB namespace.
These functions are "free" functions and do not require objects.
These VLCB namespace functions are provided for convenience and clarity.
Some are just forwarding calls to these objects while others do more
work to simplify writing module code.
These functions are described in [VLCB API documentation](../html.sketch/namespace_v_l_c_b.html)

The [example sketches](docs/Examples.md) included here are using the VLCB function style.
This is the recommended style.

## Creating a Module Descriptor File.

If you use [MMC](https://github.com/david284/MMC-SERVER) or any other
configuration tool that supports [Module Description Files](https://github.com/david284/ModuleDescriptor)
it is beneficial to create an MDF file for your developed module.