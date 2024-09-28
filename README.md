<img align="right" src="ArduinoVLCB.png"  width="150" height="75">

# VLCB-Arduino
This project implements an Arduino library for [VLCB](https://github.com/Versatile-LCB/VLCB-documents) running over CAN bus.
VLCB is an extension of [CBUS](https://www.merg.org.uk/resources/cbus).

CBUS(R) is a registered trademark of Dr. Michael Bolton.

This VLCB library code is based on a [CBUS library](https://github.com/MERG-DEV/CBUS) created by Duncan Greenwood
and extended by members of [MERG](https://www.merg.org.uk/).
See below under Credits.

See [Design documents](docs/Design.md) for how this library is structured.

There will be documentation for sketch authors that describe how to use this library.

## File Structure
This library consists of the following sub-directories and files:

src
  : This where the library source code resides.

examples
  : A few example sketches that illustrate how this library can be used.
  See [description](docs/Examples.md) of these.

docs
  : [Design documents](docs/Design.md) for the library.

Arduino
  : A set of stub header files to allow the library to be compiled outside the Arduino IDE.

test
  : Unit tests for the library. See its [README](test/README.md).

library.properties
  : Properties for exporting this library to the Arduino library manager.

CmakeLists.txt
  : CMake project definition. CMake is used for building the library outside the Arduino IDE 
    and running unit tests.

## Dependencies
Note that this library depends on a number of other libraries which must also be downloaded and included in the sketch:

* [ACAN2515](https://github.com/pierremolinaro/acan2515) - CAN bus transport using the MCP2515 controller
* [Streaming](https://github.com/janelia-arduino/Streaming) - C++ style output

## Hardware

Currently supports the MCP2515 and MCP2565 CAN controller ICs via the ACAN2515 library.

## Getting help and support

At the moment this library is still in early development and thus not fully supported.

If you have any questions or suggestions please contact the library maintainers
by email to vlcb@rosvall.ie or create an issue in GitHub.

## Building

This code may be built in multiple ways:

* Using the Arduino IDE, treating this repository as a Library.
* Directly using CMake.
* Using an IDE (e.g. VSCode) and its CMake integration.

## Credits

* Duncan Greenwood - Created the CBUS library for Arduinos which this VLCB library is based on.
* Sven Rosvall - Converted the CBUS library into this code base.
* Martin Da Costa - Contributor
* John Fletcher - Contributor
* Chris Andrews - Contributor
* David Ellis - Contributor
* brocci - Contributor to the CBUS library

## License

The code contained in this repository and the executable distributions are licensed under the terms of the
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](LICENSE.md).

If you have questions about licensing this library please contact [license@rosvall.ie](mailto:license@rosvall.ie)

## Arduino-AVR-CMake License

Portions of code in the `cmake` directory are Copyright (c) 2023 PieterP.

See the Arduino-AVR-CMake-LICENSE file in this repository.
