# Example Sketches
A few example sketches are included in the `examples` directory.
These illustrate how this library can be used.

## Common structure
All the example sketches have a structure that defines all the necessary
elements in order.

* Header files for types used in the sketch.
* definitions of module ID and version. Also defining pins for mode push button and LEDs.
* Define objects to use in the module. 
  The key object here is the controller which is the key functional point in the VLCB library.
* Module name. 
* Function setupVLCB() configures all the parameters for VLCB.
* Function setup() is used in all Arduino sketches to set up things.
  Here it is used to call setupVLCB() and any module specific setup.
* Function loop() is used in all Arduino sketches to run the Arduino.
  Here it runs the controller object to process any VLCB messages coming in and out.
  It also runs any module specific code.

Most sketches included here are configured for communication over CAN bus
using the MCP2515 transceiver.
If your module is using another transceiver change the VLCB::CAN2515 for 
a class that supports that transceiver.

## [VLCB_empty](../examples/VLCB_empty/VLCB_empty.ino)
A bare minimum sketch that only contains the necessary services 
(```MinimumNodeServiceWithDiagnostics``` and ```CANServiceWithDiagnostics```). 
It can be used to demonstrate that the Arduino module can communicate with CAN bus
and can communicate with FCU.
The FCU can assign a node number and query the module parameters.

The services used in this sketch have enabled diagnostics so that all
features of VLCB can be demonstrated.

Note that there are no support for node variables or events.

## [VLCB_1in1out](../examples/VLCB_1in1out/VLCB_1in1out.ino)
A small module that uses one input for producing events and one output that
is controlled by consumed events. 
For this it uses NodeVariableService and four services for event support.

The sketch defines one `moduleSwitch` object for an input pin 
and one `moduleLED` object for an output pin.

To manage these two I/O pins there are two functions:
* processModuleSwitchChange() queries the moduleSwitch and sends an event.
* eventhandler() is called for incoming events and controls the moduleLED.

There are two event variables:

| EV# | Function                                                                                  |
|-----|-------------------------------------------------------------------------------------------|
| 1   | Input channel number. Set to 1 to send this event when the moduleSwitch is changed        |
| 2   | Output function. Set to 1 to turn on the moduleLED. Set to 2 to make the moduleLED blink. |

The sketch supports self-consumed events. 
Create an event with EV#1 set to 1 to react to the moduleSwitch. 
Set EV#2 to make moduleLED change.
Now the LED will react when the switch is changed.

If the moduleSwitch is changed but there is no stored event with EV#1 == 1
then a default event is created with EV#1 set to 1. 
The event is created with node number of this node and an event number of 1,
or if that is already taken by another event the lowest available event number
is used.

This sketch uses services with diagnostics enabled to demonstrate them.

## [VLCB_4in4out](../examples/VLCB_4in4out/VLCB_4in4out.ino)
This is a larger sketch that handles 4 input pins and 4 output pins.
See full documentation for this sketch in its own [README](../docs/VLCB4in4out_README.md).

This sketch does not enable diagnostics to save memory on small processors
and also to demonstrate that diagnostics is optional.

## [VLCB_long_message_example](../examples/VLCB_long_message_example/VLCB_long_message_example.ino)
This is an example sketch that makes use of long messages.

## [VLCB_SerialGC_1in1out](../examples/VLCB_SerialGC_1in1out/VLCB_SerialGC_1in1out.ino)
This example sketch has the same functionality as [VLCB_1in1out](Examples.md#vlcb_1in1out) above
but uses a serial connection instead of CAN bus.
This setup is useful for testing as it does not require any CAN bus equipment
such as CANUSB or CANETHER. 
Instead, simply use a USB cable connected between a computer and the Arduino
(same cable as is used for programming the Arduino).
You can then communicate with the Arduino through the same serial port as you
use for programming the Arduino.

You can then start FCU (or any other configuration utility) and connect through
this serial port.
The FCU will then communicate with your Arduino module as a CBUS module.
