# TODO List

## Keep Node data in Controller
NodeNumber etc are split across ```Controller``` and ```Configuration```. 
Keep all access to these in ```Controller```. 
The ```Controller``` shall delegate to ```Configuration``` for persisting data.

Controller today has
* _params
* _mname

Configuration today has
* currentMode (FLiM/SLiM)
* nodeNum

Need to change the notion of SLiM/FLiM to VLCB modes.
Should be part of the MNS service.

## Update LedUserInterface class
The Minimum Node Specification describes a few more states that the LED's need to indicate. 
Review this spec again and update the user interface. 
Also need to update the interface class and add support for these states in the Controller class.

## Updates to the Storage interface
The begin method should take a size parameter. 
See calculation of this size in EepromInternalStorage.cpp. 
This calculation should sit in Controller. 
The various storage implementations may choose to ignore this size value.

## Provide access to persistent storage for user code
The Storage interface provides a simple API to persistent storage regardless of
type of storage. 
The user code should be able to reserve a chunk of bytes in this space.

## Split SerialUserInterface
The SerialUserInterface class contains everything a developer could need.
This adds code bloat. 
Split this class in two: 
  1. do the same things as the push-button does, i.e. initiate CAN enumeration, 
     initiate setup mode.
  1. all the other stuff a developer would want.

## Introduce a class/struct for CAN Frames
Currently, we are using ```CANMessage``` from the ACAN2515 library. 
This is convenient for the CAN2515 driver but for other CAN drivers it is an unnecessary
dependency.
Instead, we should introduce a CAN Frame struct that holds CAN information.
This will be used to with the CAN driver who in turn converts this to/from a data structure
type used here.

## Make CAN drivers objects instead of child of CanTransport
The relationship between CanTransport and CAN driver can be confusing as ```CanTransport```
implements some methods in the ```Transport``` interface and also introduces a some
new virtual methods that the CAN driver must implement.
Making the CAN Driver a separate object clarifies the responsibilities of ```CanTransport```
and CAN driver implementations.

## Introduce a Command Queue for communication between services
Today we have a few pieces of code where services communicate between each other.
Each of these solve the relationship between services in different ways.
1. MinimumNodeService needs to tell CanService to start CANID enumeration when changing mode.
   This is done by making a call to controller which looks up the CanService and forwards the call.
2. EventProducerService passes a produced event to CoEService which then makes it available to
   the EventConsumerService.
   This is done by including a pointer to the CoEService in the EPService and the ECService.
3. Pending pull request:Clear Events message (NNCLR) is handled by the EventTeachingService. 
   It needs to tell the EventProducerService to re-create the default events.
   The EventTeachingService raises a flag that the EventProducerService checks and if set it
   re-creates the default events.

Three different solutions to a problem with inter-service communication. We need something better.

Introduce a queue of commands.
Each command is a structure with a command (enum) and a payload that is different for each command.
The service that needs to instruct another service puts a command on the queue. 
The Controller manages the queue and passes the command on top the queue to the services in their process() call.
This command can be null if there is no command in the queue.

The impact on program memory shouldn't be too severe as a queue implementation is already kept in
the CoEService.
The existing specific code pieces for handling intra-service communication will go away.
The CoEService will go away.

### Additional Improvements
If this command queue is successful we could even use it for passing incoming messages from the Transport
to services. 
And vice versa use this queue for sending response messages and produced event messages as commands to the
Transport.

This idea can even go further and make the Transport a service (as originally envisaged).
Having the Transport as a service also allows for multiple transport connections and implement bridges
between different types of transport media such as CAN and Ethernet.

## Documentation

### Split documentation based on audience
Need to clarify which documents are intended for which audience.
There are a few distinct kinds of audiences:
  1. Developers of VLCB-Arduino library.
  2. Sketch creators that use the VLCB-Arduino library.
  3. Users of created sketches. These should probably be catered for by the sketch authors.

### How-to guide for Sketch Authors
Describe how to write a sketch with VLCB setup. 
How to create service objects and the controller object.
How to configure any parameters and service specifics.

This can only be started once we have finalized how VLCB will be set up.

## Potential bugs and opportunities for improvement

### Event lookup
Duncan use a hash value for quick search in the event table and reduce the 
number of EEPROM reads.

This code can probably be changed to loop over the hash table and for each
match with the event hash, check the EEPROM for those locations.

No need to know if there are any hash collisions in the table. 
Searching the table will be fast enough as there will be less than 255 entries, all in RAM.
