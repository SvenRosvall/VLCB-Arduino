# CanTransport Interface
```CanTransport``` serves as an interface for implementations of CAN based transports.
It handles CANID enumeration and conflict detection that would be the same for all CAN based transports. 

The service [```CanService```](Service.md#canservice) uses ```CanTransport``` derived classes
to get and send ```CANFrame``` objects.
If an incoming ```CANFrame``` contains a VLCB message then ```CanService``` will create a VlcbMessage object
and make it available to the other services.
Sending a ```VlcbMessage``` will result in a ```CANFrame``` that is sent via the ```CanTransport``` object.

available
: returns true if a ```CANFrame``` is available for retrieval.

getNextCanFrame
: get the next CAN frame that is available. 
Note that ```available()``` must be called first to ensure that a new ```CANFrame``` is available.

sendCanFrame
: send a CAN frame to the CAN bus.

