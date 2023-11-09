# Transport Interface
The ```Transport``` interface describes what methods are used by the VLCB system
and must be implemented by concrete transport classes.

The ```Transport``` interface looks like this:
```C++
class Transport
{
public:
  virtual void setController(Controller * ctrl) { }
  virtual bool available() = 0;
  virtual VlcbMessage getNextMessage() = 0;
  virtual bool sendMessage(VlcbMessage *msg) = 0;
  virtual void reset() = 0;
};
```

The methods in this interface are:

setController
: set a pointer to the controller object in the implementing class. 
There is a default implementation here so the implementing class does not need to implement
it if it doesn't need a pointer to the controller.

available
: returns true if there is a message available.

getNextMessage
: retrieves the next message that is available.

sendMessage
: send a message to the transport mechanism being implemented.

reset
: reset the transport mechanism. This is called to restart a session or to fix problems.

## CanTransport
The class ```CanTransport``` serves as a base class for implementations of CAN based transports.
It handles CANID enumeration and conflict detection that would be the same for all CAN based transports. 

```CanTransport``` implements the methods ```getNextMessage``` and ```sendMessage``` above. 
However, it requires that any concrete implementing classes implement the following methods:

getNextCanMessage
: get the next CAN message that is available.

sendCanMessage
: send a CAN message to the CAN bus.

Note that these two methods work with a CAN message (```CANMessage```) rather than a 
normal message (```VlcbMessage```) that is used in the rest of the VLCB implementation.
The class ```CanTransport``` will translate between these two message types.