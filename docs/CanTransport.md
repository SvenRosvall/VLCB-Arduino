# CanTransport Interface
```CanTransport``` serves as an interface for implementations of CAN based transports.
It handles CANID enumeration and conflict detection that would be the same for all CAN based transports. 

The service [```CanService```](Service.md#canservice) uses ```CanTransport``` derived classes
to get and send ```CANFrame``` objects.
If an incoming ```CANFrame``` contains a VLCB message then ```CanService``` will create a VlcbMessage object
and make it available to the other services.
Sending a ```VlcbMessage``` will result in a ```CANFrame``` that is sent via the ```CanTransport``` object.

Implementations of ```CanTransport``` must implement the following methods:

available()
: returns true if a ```CANFrame``` is available for retrieval.

getNextCanFrame()
: get the next CAN frame that is available. 
Note that ```available()``` must be called first to ensure that a new ```CANFrame``` is available.

sendCanFrame()
: send a CAN frame to the CAN bus.

## Implementations

This library provides the following concrete transport classes:

CAN2515
: Implementation for using the MCP2515 CAN transceiver.

SerialGC
: Use the GridConnect protocol for sending CAN frames over a serial connection.

The following concrete transports exist externally.

[VCAN2040](https://github.com/MartinDaCosta53/VCAN2040)
: Implementation for Raspberry Pi Pico using a software CAN transceiver.
