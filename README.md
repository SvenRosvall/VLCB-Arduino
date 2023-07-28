<img align="right" src="arduino_cbus_logo.png"  width="150" height="75">

# VLCB-Arduino
Arduino library for [VLCB](https://github.com/Versatile-LCB/VLCB-documents) running over CAN bus.

The code is based on a CBUS library created by Duncan Greenwood.

See [Design documents](docs/Design.md) for how this library is structured.

Note that this library depends on a number of other libraries which must also be downloaded and included in the sketch:

* [ACAN2515](https://github.com/pierremolinaro/acan2515) - CAN bus transport using the MCP2515 controller
* [Streaming](https://github.com/janelia-arduino/Streaming)		- C++ style output

## Hardware

Currently supports the MCP2515 and MCP25625 CAN controller ICs via the ACAN2515 library

## Credits

* Duncan Greenwood - Created the CBUS library for Arduinos which this VLCB library is based on.
* Sven Rosvall - Converted the CBUS library into this code base.
* Martin Da Costa - Contributor
* John Fletcher - Contributor
* brocci - Contributor

## License

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
