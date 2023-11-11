# Minimum Node Service

The Minimum Node Service provides the basic VLCB interface and functions for the library. As such,
it is the only service whose inclusion is mandatory.

The interface looks like this:
```
class MinimumNodeService : public Service
{
public:
  virtual void setController(Controller *cntrl) override;
  virtual void process(UserInterface::RequestedAction requestedAction) override; 
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override;

  virtual byte getServiceID() override { return SERVICE_ID_MNS; }
  virtual byte getServiceVersionID() override { return 1; }
  
  virtual void begin() override;
  
  // backdoors for testing
  void setHeartBeat(bool f) { noHeartbeat = !f; }
  void setSetupMode();
  void setUninitialised();
};
```

## Operating Mode

A newly programmed module will be in Uninitialised Mode and will have default node number of 0x0000.
Consequently, it is not possible to communicate with the module using VLCB. The module needs to be 
placed in Setup Mode whilst connected to an FCU, or similar. This is done by using the User 
Interface (UI) button or some similar direct connection UI.

The module can now communicate and negotiate with the FCU to obtain a Node Number and any bus 
specific information (if CAN is the transport medium, then this will be a unique CAN ID).

At the end of this process, the module will be in Normal Mode. If there is any problem during the
process, the module will sen error codes to the FCU indicating the point in the process where the
problem arose.  If the FCU doesn't respond within 30 seconds, the module will revert to
Uninitialised Mode. 

### Allocating a Node Number
Upon entering SETUP MODE, the module shall send OPC_RQNN. In a ‘virgin’ node the accompanying NN
will be 00 00 but if the node already has a NN, this will be included instead. 

A software tool may request the the node parameters at this stage as a block by sending OPC_RQNP
in which case the module responds with OPC_PARAMS.  The software tool will then send OPC_SNN and
teh module will be set with the allocated Node Number. The module will then send OPC_NNACK with
the new Node Number to acknowledge receipt. The module shall then be set to NORMAL MODE.

### Reverting to UNINITIALISED MODE

Whilst it is possible to re-initialise a module thereby getting a new Node Number, it is not
possible to revert directly to UNINITIALISED MODE from the module. This can be done by sending
an OPC_MODE command (see below) or a factory reset from the software tool using OPC_NNRSM.

## Initialisation Op-Codes

The following op-codes are used during initialisation of the module:

### Request for Node Parameters - OPC_RQNP(0x10)

This request may be made by the software tool whilst the module is in Setup Mode and prior to
the tool issuing a node number. There is no data with this op-code and is only responded to
by a module that is in Setup Mode.  The module will respond with OPC-PARAMS followed by the
first seven parameters bytes:


| Para |                           Description                                   |
|------|-------------------------------------------------------------------------|
|  1   |The manufacturer ID as a HEX numeric                                     |
|      |(If the manufacturer has a NMRA number this can be used)                 |
|  2   |Minor code version as an alphabetic character (ASCII)                    |
|  3   |Manufacturer’s module identifier as a HEX numeric                        |
|  4   |Number of supported events as a HEX numeric                              |
|  5   |Number of Event Variables per event as a HEX numeric                     |
|  6   |Number of supported Node Variables as a HEX numeric                      |
|  7   |Major version as a HEX numeric. (can be 0 if no major version allocated) |


### Request Module Name - OPC_RQMN(0x11)

Requests the name of the module that is in Setup Mode. That module will respond with
OPC_NAME(0xE2) to include the seven character name.

### Set Node Number - OPC_SNN(0x42)

Sets the moduel Node Number whilst it is Setup Mode

### Request Node Number - OPC_RQNN(0x50)

This is a request from a module that has entered Setup Mode for a Node Number.  If this 
module is in Setuo Mode, this is a system fault condition in that only one module can be 
in Setup Mode at any one time.  If this module sees this op-code whilst in Setup Mode, it
aborts from Setup Mode.

## Operational Op-Codes

The following Op-Codes are used when a module is in Normal Mode:

### Query Node Number - OPC_QNN(0x0D)

Requests an OPC_PNN response.  This is a broadcast message and so will elicit a response from
every node on the network.  The OPC_QNN(0xB6) response is the Node Number (2 bytes), 
the Manufacturers ID (1 byte), the ModuleID )1 byte) and the Moduel Flags (1 byte).  The flags
are:

| Bit |                Description                          |
|-----|-----------------------------------------------------|
|  0  | Set to 1 for consumer node                          |
|  1  | Set to 1 for producer node                          |
|  2  | Set to 1 for Normal Mode                            |
|  3  | Set to 1 for Bootloader compatible                  |
|  4  | Set to 1 if able to consume its own produced events |
|  5  | Set to 1 if module is in Learn mode                 |
|  6  | Set to 1 if module supports service discovery       |

### Reset to Manufacturers Defaults - OPC_NNRSM(0x4F)

The module sends a Node Number release request (OPC_NNREL) and then resets to the 
manufacturer's default settings.


### Software Reset - OPC_NNRST(0x5E)

Resets the module's microprocessr without changing any settings.

### Request Node Parameters by Index - OPC_RQNPN(0x73)

Sends a OPC_PARAN(0x9B) with the specific parameter value requested.  The exception is
if the parameter inex is zero in which case a PARAN message is sent with the numebr of 
parameters available followed by a sequence of PARAN messages, one for each available
parameter.

### Set Operating Mode - OPC_MODE(0x76)

This op-code carries a command code each of which sets a specific operating state.
Command values 0x00 to 0x07 and 0xFF are reserved for transition between exclusive states
and are managed within this service. Command values 0x08 to 0xFE are available for service
use. To this end, if a command value is not applicable to the Minimum Node Service, the 
Controller will offer it, in sequence, to other services.

The following caommand values are used by the Minimum Node Service:

|    Value     |   Type    |        Request Command        |
|--------------|-----------|-------------------------------|
|    0xFF      | Exclusive | Change to Unitialised Request |
|    0x00      | State     | Change to Setup Request       |
|    0x01      | Changes   | Change to Normal Mode         |
| 0x02 to 0x07 |           | Reserved                      |
|--------------|-----------|-------------------------------|
|    0x0C      | Service   | Turn on Heartbeat             |
|    0x0D      | Usage     | Turn off Heartbeat            |

Service Usage Command Code Turn on Heartbeat will cause a OPC_HEARTB(0xAB) to be sent every
5000mS.  Turn of Heartbeat will stop the transmission of OPC_HEARTB.

### Request Service Discovery = OPC_RQSD(0x78)

If the Service Index is zero, then the module will send OPC_SD(0xAC) with the number of 
services supported.  It will then send an OPC_SD for each of those services.

If the Service Index is greater than zero, then a single OPC_ESD(0xE7) will be sent relating
to that specific service.

### Request Diagnostic Data - OPC_RDGN(0x87)

If the Service Index is zero, then the module will send OPC_DGN(0xAC) with the number of 
services supported.  It will then send an OPC_DGN for each of those services.

If the Service Index is greater than zero, then a single OPC_DGN(0xAC) will be sent relating
to that specific service.

## User Sketch

A user sketch needs to set up the required VLCB objects and then call ```VLCB.process()``` from 
the main loop.

The include files may look like this:
```
// VLCB library header files
#include <Controller.h>               // Controller class
#include <Configuration.h>            // module configuration
#include <Parameters.h>               // VLCB parameters
#include <vlcbdefs.hpp>               // VLCB constants
#include <LEDUserInterface.h>
#include "MinimumNodeService.h"
```
This represents the minimum, mandatory libraries.  To this list optional libraries need to be
added for Transport service and other services required, such as Event Consumer.

The setup code may look like:
```
// Controller objects
VLCB::Configuration modconfig;               // configuration object
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::MinimumNodeService mnService;
VLCB::Controller controller( &modconfig, ledUserInterface,
                            { &mnService }); // Controller object


setup()
{
  See examples for what needs to be added depending upon services included.
}
```