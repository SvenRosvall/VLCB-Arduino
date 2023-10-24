# TODO List

## Keep Node data in Controller
NodeNumber etc are split across Controller and Configuration. 
Keep all access to these in Controller. 
The Controller shall delegate to Configuration for persisting data.

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

## Potential bugs and opportunities for improvement

### Make CANFrame Generic
The struct CANFrame is currently a copy of CANMessage from ACAN2515. 
CANFrame doesn't need the CAN specific fields such as ```id```, ```ext``` and ```rtr```.
These fields can be populated within the transport class.
```id``` is already populated within the CAN2515 class with the value from module configuration.

Once this refactoring is done, CANFrame can be renamed to something more generic
such as ```VlcbMessage```.

The CAN specifics are exposed to some services. 
* CanService checks if RTR is set and checks for CANID collisions.
* LongMessageService relies on CANID for stringing messages together.

### Event lookup
Duncan use a hash value for quick search in the event table and reduce the 
number of EEPROM reads.

This code can probably be changed to loop over the hash table and for each
match with the event hash, check the EEPROM for those locations.

No need to know if there are any hash collisions in the table. 
Searching the table will be fast enough as there will be less than 255 entries, all in RAM.
