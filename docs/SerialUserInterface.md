# Serial User Interface
This library provides a means of communicating with the module using its serial
interface.  The functions allow the VLCB switch and LEDs to be by-passed or not
included at all.  Additional functions that may be of use during development
are also available.

## Serial Monitor

By sending a single character to the Arduino using the serial send facility in the
serial monitor of the Arduino IDE (or similar),it is possible to initiate certain operations
or get information from the Arduino sketch.

#### 'n'
This character will return the node configuration.

#### 'e'
This character will return the learned event table in the EEPROM.

#### 'v'
This character will return the node variables.

#### 'c'
This character will return the CAN bus status.

#### 'h'
This character will return the event hash table.

#### 'm'
This character will return the amount of free memory. 

#### 'r' (implementation pending)
This character will cause the module to renegotiate its VLCB status by requesting a node number.
The FCU will respond as it would for any other unrecognised module.

