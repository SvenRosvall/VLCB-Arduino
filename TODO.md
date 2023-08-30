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
The read and write methods have an "EEPROM" suffix. 
Remove this as not all storages are EEPROM based.

The begin method should take a size parameter. 
See calculation of this size in EepromInternalStorage.cpp. 
This calculation should sit in Controller. 
The various storage implementations may choose to ignore this size value.

## Provide access to persistent storage for user code
The Storage interface provides a simple API to persistent storage regardless of
type of storage. 
The user code should be able to reserve a chunk of bytes in this space.
