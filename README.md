<img align="right" src="arduino_cbus_logo.png"  width="150" height="75">

# VLCB-Arduino
Arduino library for VLCB running over CAN bus

The code is based on a Controller library created by Duncan Greenwood.

See [Design documents](docs/Design.md) for how this library is structured.

Note that this library depends on a number of other libraries which must also be downloaded and included in the sketch:

* ACAN2515		- concrete implentation of the Controller class using the MCP2515 controller
* Streaming		- C++ style output

## Hardware

Currently supports the MCP2515 and MCP25625 CAN controller ICs via the ACAN2515 library

## License

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
