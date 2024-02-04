# TODO List

## UI Service ID shall be ignored
UI classes are implemented as services. However they are not services in VLCB sense.
Therefore the UI services shall not be included in the response to RQSD.

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
  1. all the other stuff a developer would want, such as transport statistics and
     resetting the module.

## Move CAN2515 to its own package/repo
Now that we are getting more CAN transport implementations it is time to move CAN2515 
away into its own library. 
This breaks the dependency on ACAN2515 which should not be required for other CAN transports.

## Ideas for reducing memory usage
### Make CanFrame polymorphic
Use virtual functions to get each field.
Each CAN transport implementation must implement CAN Frame functions.

This should reduce the amount of copying between CanFrame and the datastructure used 
within the CAN transport implementation.

However, the added coding and memory used for virtual functions may negate the benefits 
of a polymorphic CanFrame class.

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
