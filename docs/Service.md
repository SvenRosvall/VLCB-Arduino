# Service Interface
The ```Service``` interface describes what methods are used by the VLCB system
and must be implemented by concrete service classes.

The VLCB system is configured with a set of services that implement the functionality
needed by the VLCB module.
A list of services provided in this VLCB library are listed in the section [Provided Services](#provided-services).

User defined services are encouraged to implement new functionality that is not covered
by the provided services

The ```Service``` interface looks like this:
```C++
class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual void begin() {}
  virtual byte getServiceID() = 0;
  virtual byte getServiceVersionID() = 0;

  virtual void process(Action * action) = 0;
};
```

The methods in this interface are:

setController
: set a pointer to the controller object in the implementing class.
There is a default implementation here so the implementing class does not need to implement
it if it doesn't need a pointer to the controller.

begin
: This optional method is called at the beginning of the Arduino sketch.
Define this method for the service to do any setup required at the beginning. 

getServiceID
: Shall return a unique ID for this service.
This ID is used by configuration utilities to identify the service type.

getServiceVersionID
: Shall return the version of the implementation of this service.
This version is used by configuration utilities to identify any updated features in the
service. 
There is no need to bump up the version number for minor changes and bug fixes. 

process
: This method that be called regularly. 
It has a pointer to a Action that needs to be processed or a null pointer if there is
no Action to be processed.
Use this for any processing that needs to be performed now and then such as polling for
changes of input pins.


## Services provided in this VLCB library

### MinimumNodeService
Handles the OP-codes for managing nodes.
It is required for use in a VLCB module.

### CanService
This is a service that is specifically for modules on a CAN bus and is required for such modules.
Handles OP-codes CANID and ENUM for re-assigning CANID for the module.

The ```CanService``` works in tandem with a CAN transport object, derived from the 
[```CanTransport```](CanTransport.md) interface, which must be provided as an argument to its constructor.

### NodeVariableService
Handles configuration of node variables for the module.

### EventConsumerService
Handles incoming events.
To use this the module application needs to register an event handler function with
```setEventHandler()```.
The signature of the handler function shall be
```C++
void handler(byte index, const VlcbMessage *msg);
```
The arguments provided to the handler function are:
* ```index``` : The index number of the incoming event in the event table.
* ```msg``` : The message structure that contains the event.

### EventProducerService
Facilitates sending events. 
Call the ```sendEvent()``` from the application code.
The parameters should be
* ```state``` : A flag to say if the event is an on or off event.
* ```index``` : An index for the produced event that relates to the event table entry.
* ```data1``` - ```data3``` : Up to three data bytes to go with the event.

### EventTeachingService
Facilitates teaching events. 
Both consumed and produced events can be taught.

### ConsumeOwnEventsService
This service facilitates sent events to be received by the same module.

### LedUserInterface
Manages the green and yellow LEDs and also the push button on the VLCB module.
Updates the LEDs based on activities on the module. 
When the push button is pressed sends an Action to tell the other services that the
user has requested some action.

### SerialUserInterface
Provides a user interface on the serial port on the Arduino.
Prints status messages and also handles actions the user enters on the serial port.
See more details in [SerialUserInterface](SerialUserInterface.md) documentation.