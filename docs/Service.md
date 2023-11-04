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

  virtual void process(UserInterface::RequestedAction requestedAction) {}
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) = 0;
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
: This is an optional method that will be called regularly.
Use this for any processing that needs to be performed now and then such as polling for
changes of input pins.

handleMessage
: Handle an incoming message.
The op-code is provided to help the service deciding what to do for the message.
Return a value ```PROCESSED``` if the message was handled and no other services need
to see this message. 
Otherwise, return ```NOT_PROCESSED``` so that the system knows that this message was not
processed and other services shall get a chance to process this message.


## Services provided in this VLCB library

### MinimumNodeService
Handles the OP-codes for managing nodes.
It is required for use in a VLCB module.

### CanService
This is a service that is specifically for modules on a CAN bus and is required for such modules.
Handles OP-codes CANID and ENUM for re-assigning CANID for the module.

The ```CanService``` works in tandem with a CAN transport object, derived from the 
```CanTransport``` class, which must be provided as an argument to its constructor.

### NodeVariableService
Handles configuration of node variables for the module.

### EventConsumerService
Handles incoming events.
To use this the module application needs to register an event handler function with
```setEventHandler()```.
The signature of the handler function shall be one of
```C++
void handler(byte index, VlcbMessage *msg);
void handler(byte index, VlcbMessage *msg, bool ison, byte evval);
```
The arguments provided to the handler function are:
* ```index``` : The index number of the incoming event in the event table.
* ```msg``` : The message structure that contains the event.
* ```ison``` : A flag to indicate if the event is an on or off event.
* ```evval``` : The value of event variable 1 for the incoming event.

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
Simply add a pointer to this service to the constructors of the EventConsumerService
and the EventProducerService. 