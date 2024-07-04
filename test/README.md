# Off-line Testing
To make it easier to develop the library a test suite has been created so that the library
can be tested without an Arduino.
This allows for testing most of the library using your favourite C++ development environment.

## Build and Run Tests
The library includes a CMakeLists.txt file that describes how to compile and link the
library and test source code. 
Run the following commands to run the tests: 
```
$ cmake .
$ make testAll
$ ./testAll
```

## Mocking the Arduino Library
The Arduino IDE includes a huge library and there are many libraries that can be added.
These libraries are written to run on Arduino hardware.
To run this test suite outside Arduinos we need a 'mock' library that looks like the real
Arduino libraries but doesn't require the Arduino hardware.

A set of header files are provided in the `Arduino` directory. 
Use these for off-line testing instead of the real Arduino library headers.
These mock header files do not contain any implementation.
They are only used to make it possible to compile this library outside the Arduino IDE.

Within the `test` directory we have `ArduinoMock.hpp` and `ArduinoMock.cpp` which contains 
implementations of the function calls that are mocked in the `Arduino` directory headers.

There are a few supporting function calls:
`void setAnalogRead(int pin, int value)`
  : Sets a value that the next call to `analogRead(pin)` shall return.

`void setDigitalRead(int pin, PinState value)`
  : Sets a value that the next call to `digitalRead(pin)` shall return.

`int getAnalogWrite(int pin)`
  : Gets the value written to a pin with `analogWrite(pin)`

`PinState getDigitalWrite(int pin)`
  : Gets the value written to a pin with `digitalWrite(pin)`

`void addMillis(unsigned long millis)`
  : Adds `millis` to the mock clock used for testing. 
    The call `millis()` will return this mock clock value.
    Use this to simulate elapsed time and testing timouts.

`void clearArduinoValues()`
  : Clear all the internal data kept by the functions above. 
    This prepares this internal data for a new unit test run.

## Mocking Hardware Classes
Thanks to the use of interface classes in the [library design](../docs/Design.md)
it is possible to create mock classes for classes that are coded against the Arduino hardware.
These mock classes pretend to handle this hardware but can be instrumented to return 
pre-defined values that will be useful for unit testing other classes that depend on these
interfaces.

`MockStorage`
: Implements the `Storage` interface.
This mocks out the persistent storage.

`MockCanTransport`
  : Implements the `CanTransport` interface. 
  It captures `CANFrame` objects being sent and can be instrumented with `CANFrame` objects to be returned.

`MockTransportService`
  : Implements a transport service that handles sending and receiving `VlcbMessage` objects
  and processing `Action` objects. 
  This is used to replace the `CanService` when testing other services of the library 
  and there is no need to deal with CAN specific code.

`MockUserInterface`
  : Implements a user interface service that captures `Action` objects that deal with user
  interaction.

## Test Tools
`TestTools.hpp` contains a few macros that can be used in unit tests:

`test()`
: Add this to each unit test. This prints out the file name and function name for tracing 
the current test.
It also clears all the internal mock data.

`assertEquals(expected, actual)`
: If `expected` and `actual` values differ then print out an error message together
with file name and line number to make it easy to find what test has failed.

## Unit Tests
Each C++ test file tests one class. It can contain functions that make up one unit test each.
Each unit test shall start with `test()` and then create an object of the class to be tested.
The unit test then calls some function of that class and then checks returned values and/or
state within the tested object with `assertEquals()`.