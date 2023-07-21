<img align="right" src="arduino_cbus_logo.png"  width="150" height="75">

# VLCVB-Arduino
Arduino library for VLCB running over CAN bus

The code is based on a CBUS library created by Duncan Greenwood.

It provides everything required to implement a FLiM CBUS module that can be configured using FCU or JMRI.

Note that this library depends on a number of other libraries which must also be downloaded and included in the sketch:

ACAN2515		- concrete implentation of the CBUS class using the MCP2515 controller
Streaming		- C++ style output

## Hardware

Currently supports the MCP2515 and MCP25625 CAN controller ICs via the ACAN2515 library

## License

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
