
# Arduino library for MERG CBUS running over CAN bus

A library that implements the abstract CBUS base class. It contains methods to support the MCP2515/25625 CAN controller IC

Note that this library depends on a number of other libraries which must also be included in the sketch:

CBUS 			- abstract CBUS base class
ACAN2515		- concrete implentation of the CBUS class using the MCP2515 controller
CBUSswitch
CBUSLED
CBUSconfig
Streaming		- C++ style output

## Hardware

Currently supports the MCP2515 or MCP25625 CAN controller chips via the ACAN2515 library

## Documentation

See the included example sketch

## License

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
