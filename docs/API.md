# VLCB API

This VLCB library is built with an object-oriented architecture
described in the [design document](Design.md).
It uses a `Controller` object for the core tasks, a `Configuration`
object for storing module configurations and a set of [services](Service.md).

The VLCB module code can be written in two different styles.
One style is to define these objects and calling member functions of these objects.

The other style is to use functions in the VLCB namespace.
These functions are "free" functions and do not require objects.
These VLCB namespace functions are provided for convenience and clarity.
Some are just forwarding calls to these objects while others do more
work to simplify writing module code.

The example sketches are using the VLCB function style.

## VLCB functions
These functions are in the VLCB namespace to avoid name clashes with
other libraries.
Prefix each call with `VLCB::` or add a `using namespace VLCB;` 
directive at the top of your code file.

### Startup functions
* `void checkStartupAction(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)` \
This is a helper function that can be called at the beginning of the
`setup()` function.
It performs a few checks at startup of the module.
At the moment does:
  1. Checks if the push button is pressed. If so it will reset the module to factory settings, i.e. write all EEPROM with 0xFF bytes.


* `void begin()` \
Call this function in the `setup()` function, after all configuration 
is complete.
This initialises the internals of the VLCB library.

### Module configuration
* `void setServices(std::initializer_list<Service *> services)` \
Set up the services to be used by the module.
These services must be defined above in the code, outside any functions.
The used services shall be listed with curly brackets, called an "initializer list".
A call will look like this:
```
    setServices({service1, service2, ...});
```

* `void setVersion(char maj, char min, char patch)` \
Set major, minor and patch versions for the module.
Note that `maj` and `patch` versions are numbers while `min` is a letter. 


* `void setModuleId(byte manu, byte moduleId)` \
Set the module identification for the module.
`manu` shall either be one of:  
  * `MANU_MERG` (165) for modules that are shared and have a `moduleId`
    that is registered within the VLCBDEFS repository to ensure that it is
    globally unique.
  * `MANU_DEV` (value 13) for private modules where the `moduleId` is
    not registered in VLCBDEFS. The `moduleId` only needs to be unique
    within the network of the developer.


* `void setName(char *mname)` \
Set the name of the module.
The name shall be a string of max 7 characters.


* `void setNumNodeVariables(byte n)` \
Set number of node variables that the module will use.
@param n number of node variables.


* `void setEventsStart(byte n)` \
_Optional_: Sets the address where event data starts in the EEPROM. 


* `void setMaxEvents(byte n)` \
Set the max number of events the module can handle.


* `void setNumEventVariables(byte n)` \
Set the number of event variables that are used by each stored event. 


### Module configuration access
* `VlcbModeParams getCurrentMode()` \
* `byte getCANID()` \
* `unsigned int getNodeNum()` \
* `unsigned int getFreeEEPROMbase()` \
* `byte readNV(byte nv)` \
* `void writeNV(byte nv, byte val)` \
* `byte getEventEVval(byte idx, byte evnum)` \
* `byte findExistingEventByEv(int evIndex, byte value)` \
* `byte findExistingEvent(unsigned int nn, unsigned int en)` \
* `bool isEventIndexValid(byte eventIndex)` \
* `bool doesEventExistAtIndex(byte eventIndex)` \
* `byte findEmptyEventSpace()` \
* `void createEventAtIndex(byte eventIndex, unsigned int nn, unsigned int en)` \
* `void writeEventVariable(byte eventIndex, byte evIndex, byte value)` \

* `bool sendMessageWithNN(VlcbOpCodes opc)` \
* `bool sendMessageWithNN(VlcbOpCodes opc, byte b1)` \
* `bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2)` \
* `bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3)` \
* `bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4)` \
* `bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4, byte b5)` \

* `void resetModule()` \

### Running
* `void process()` \
Call this in the `loop()`function.
It lets the VLCB library core execute its tasks such as check for new
incoming messages and run queued tasks.