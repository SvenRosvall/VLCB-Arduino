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
