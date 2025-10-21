# Design Overview
This document describes the main components within the VLCB library and how they interact.


This VLCB library is based on Duncan Greenwood's [CBUS library](https://github.com/MERG-DEV/CBUS)
and extended with VLCB specific features.

## High Level Architecture
The code is organised as a central controller object that controls functionality
via a storage object and a set of service objects.

Services implement various groups of functionality within VLCB such as events or DCC control.
The user sketch can select the set of services needed to provide the necessary functionality
for the VLCB module that is created.
There are also services for user interfaces and communication over different transports such as CAN, Wi-Fi and BLE.
Currently only a CAN transport using the CAN2515 chip is included in the library.

The library supports storage for node variables and event variables in EEPROM or Flash memory.
A Configuration object controls persistence of node specific data such as parameter, node variables
and events using the chosen storage.

A class diagram is shown below with a selection of concrete implementations of transports, storage, 
and services.
![Class Diagram](VLCB-Arduino-Classes.png)

### Details
* There is no message queuing in this diagram. Message queuing is expected to happen in the transport layer.
  Some transport hardware has message queues builtin, for others the transport implementation
  will include queues.

## Workflow
The Controller maintains a ```Action``` bus, which is implemented as a circular buffer.
Each service can put actions on this bus and act on actions placed there by other services.
An action represents tasks or information to be shared with other services. 
The Action bus decouples services from each other and makes it easier to add new services.

The main workflow is that the VLCB Controller object runs every so often from the sketch loop() function.
During each iteration the controller calls out to each service to do any processing it needs to do. 
The top element on the action bus (if any) is included in this call.

The ```CanService``` checks for incoming messages on the CAN bus and if the action object passed
from the controller is an outgoing message it sends it to the CAN bus.

The ```EventConsumerService``` may react to consumed events by calling a user registered callback so that
the user sketch can act on this event for example to turn on an LED or move a servo.

The user sketch may produce events that are managed by the ```EventProducerService``` object which then passes
the event as an Action via the Controller to the transport object.

### Dataflow
Most of the VLCB functionality uses a message object ```VlcbMessage``` that passes incoming and
outgoing messages around via the Action bus. 
The ```VlcbMessage``` object contains 8 bytes where the first is the op-code and the remaining 7 bytes
are any optional data bytes for that op-code.

The ```CanService``` translates the ```VlcbMessage``` to/from a ```CANFrame``` class that contains 
CAN specific fields and a ```VlcbMessage``` in its data portion.

The ```CanTransport``` object translates the ```CANFrame``` object to and from an object that represents
a CAN frame containing an id, 8 bytes of data (same as the VlcbMessage) and the flags
```rtr``` and ```ext```.
This CAN frame is then passed to or from a CAN driver, such as CAN2515. 
CAN drivers may need to convert this CAN frame to a data structure used by any library
that is used by that driver.

## Configuration
The Configuration object stores node variables (NV) and event variables(EV) and any other configuration
that is required. It makes use of a storage object that has different implementations for different
storage types. Not all Arduino modules have EEPROM or enough EEPROM, in which case external EEPROM or
Flash memory can be used.
See further details in [Persistent Storage](PersistentStorage.md) documentation.

The configuration support is divided into a Configuration object that manages NVs and EVs.
It provides functions that deal with these NVs and EVs. 
It makes use of a Storage interface where implementing classes store the data at a given
address.

There are a few implementing storage classes:

EepromInternalStorage
: Stores data in EEPROM directly on the processor.

EepromExternalStorage
: Stores data in external EEPROM connected via I2C.

DueEepromEmulationStorage
: Support by emulating EEPROM for the Arduino DUE.

FlashStorage
: Stores data in Flash memory. Useful for modules that do not have onboard EEPROM or too
little EEPROM.

## Services

The interpretation of incoming messages is handled by a set of services.
VLCB offers up a message to each service in turn. 
If a service is able to handle that message no further services will be offered the message.
Thus, the order of configured services is important.

Read more about the ```Service``` interface and how services work in 
[Service documentation](Service.md).

Examples of some services:

MinimumNodeService
: Handles all the mandatory op-codes that all VLCB modules must implement. 
These op-codes involve running modes and basic node configuration such as node number and
module parameters.

EventConsumerService
: Handles incoming events that shall result in actions on the module.
If the COE parameter is set it will also handle outgoing events.

ConsumeOwnEventsService
: Enables passing events produced by the producer service back to the consumer service.
It doesn't do anything else than setting the COE parameter flag which also tells
the EventConsumerService to also look for outgoing events on the action bus.

LongMessageService
: Handles the long message extension to CBUS as defined in RFC005.

LEDUserInterface
: Implements a low level UI using a push button, a green LED and a yellow LED.

It will be possible to implement new user interfaces that make use of, for example, an OLED screen or simply
use the USB connection for serial communication.

## Diagnostics

Diagnostics are optional. 
The implementation of diagnostics uses substantial amount of memory which
might not be available in smaller processors.
This library provides a choice to enable or omit diagnostics by using a
variation of service classes.

The services listed above do not have diagnostics included.
To enable diagnostics choose service classes with a "WithDiagnostics" suffix.
E.g. The CAN service class ```CanService``` does not provide diagnostics
while the class ```CanServiceWithDiagnostics``` does provide diagnostics.

Note: Not all services have a diagnostics enabled counterpart yet.

## User Sketch

A user sketch needs to set up the required VLCB objects and then call ```VLCB.process()``` from 
the main loop.

The setup code may look like:
```
// Global definitions
VLCB::LEDUserInterface userInterface(greenLedPin, yellowLedPin, pushButtonPin); 
VLCB::CAN2515 can2515(interruptPin, csPin); 
VLCB::CanService canService(&can2515);
VLCB::MnsService mnsService;
VLCB::EventConsumerService eventConsumerService(myActionCallback);
VLCB::Configuration config;
VLCB::Controller moduleController(&config, {mnsService, userInterface, canService, eventConsumerService});

setup()
{
  // set config layout parameters
  modconfig.setNumNodeVariables(10);
  modconfig.setNumEvents(32);
  modconfig.EE_PRODUCED_EVENTS = 1;
  modconfig.setNumEVs(2);

  // set module parameters
  modconfig.getParams().setVersion(VER_MAJ, VER_MIN, VER_BETA);
  modconfig.getParams().setManufacturer(MANUFACTURER);
  modconfig.getParams().setModuleId(MODULE_ID);  
  modconfig.setName(mname);

  can2515.setNumBuffers(2);
  can2515.setOscFreq(OSC_FREQ);
  can2515.begin();
  
  controller.begin();
}
```
See also the example sketches how to use the VLCB library.

### Convenience functions
A set of convenience functions have been introduced to remove some
complexity.
```
VLCB::CAN2515 can2515;                  // CAN transport object

// Service objects
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::MinimumNodeServiceWithDiagnostics mnService;
VLCB::CanServiceWithDiagnostics canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::ConsumeOwnEventsService coeService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;

//
/// setup VLCB - runs once at power on from setup()
//
void setupVLCB()
{
  VLCB::checkStartupAction(LED_GRN, LED_YLW, SWITCH0);

  VLCB::setServices({
    &mnService, &ledUserInterface, &canService, &nvService,
    &ecService, &epService, &etService, &coeService});

  // set config layout parameters
  VLCB::setNumNodeVariables(10);
  VLCB::setMaxEvents(32);
  VLCB::setNumProducedEvents(1);
  VLCB::setNumEventVariables(2);

  // set module parameters
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BETA);
  VLCB::setModuleId(MANUFACTURER, MODULE_ID);

  // set module name
  VLCB::setName(mname);

  can2515.setNumBuffers(2);
  can2515.setOscFreq(OSC_FREQ);
  can2515.begin();
  
  VLCB::begin();
}
```