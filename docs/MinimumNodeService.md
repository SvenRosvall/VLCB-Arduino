# Minimum Node Service

The Minimum Node Service provides the basic VLCB interface and functions for the library. As such,
it is the only service whose inclusion is mandatory.

Detailed documentation is available in [MinimumNodeService API definition](../html.library/class_v_l_c_b_1_1_minimum_node_service.html)

## Operating Mode

A newly programmed module will be in Uninitialised Mode and will have default node number of 0x0000.
Consequently, it is not possible to communicate with the module using VLCB. The module needs to be 
placed in Setup Mode whilst connected to an FCU, or similar. This is done by using the User 
Interface (UI) button or some similar direct connection UI.

The module can now communicate and negotiate with the FCU to obtain a Node Number and any bus 
specific information (if CAN is the transport medium, then this will be a unique CAN ID).

At the end of this process, the module will be in Normal Mode. If there is any problem during the
process, the module will send error codes to the FCU indicating the point in the process where the
problem arose.  If the FCU doesn't respond within 30 seconds, the module will revert to
Uninitialised Mode. 

This process using the FCU follows a defined communication sequence. This sequence is defined in
the MinimumNode Service Document, the CBUS(R) Developers Guide and the FCU user guide.  It is the
module developer's and users responsibility to ensure that any software setup tool meets these
requirements.

## Diagnostics

Diagnostics is optional and can be enabled by using the class ```MinimumNodeServiceWithDiagnostics```.

## Operational Op-Codes

The following Op-Codes are used when a module is in Normal Mode:

### Query Node Number - OPC_QNN(0x0D)

Requests an OPC_PNN response.  This is a broadcast message and so will elicit a response from
every node on the network.  The OPC_QNN(0xB6) response is the Node Number (2 bytes), 
the Manufacturers ID (1 byte), the ModuleID (1 byte) and the Module Flags (1 byte).  The flags
are specified in the vlcbdefs.h file

### Reset to Manufacturers Defaults - OPC_NNRSM(0x4F)

The module sends a Node Number release request (OPC_NNREL) and then resets to the 
manufacturer's default settings.

### Software Reset - OPC_NNRST(0x5E)

Resets the module's microprocessor without changing any settings.

### Request Node Parameters by Index - OPC_RQNPN(0x73)

Sends a OPC_PARAN(0x9B) with the specific parameter value requested.  The exception is
if the parameter index is zero in which case a PARAN message is sent with the number of 
parameters available followed by a sequence of PARAN messages, one for each available
parameter.

### Set Operating Mode - OPC_MODE(0x76)

This op-code carries a command code each of which sets a specific operating state.
Command values 0x00 to 0x07 and 0xFF are reserved for transition between exclusive states
and are managed within this service. Command values 0x08 to 0xFE are available for service
use. To this end, if a command value is not applicable to the Minimum Node Service, the 
Controller will offer it, in sequence, to other services.

The following command values are used by the Minimum Node Service:

|    Value     |   Type    | Request Command                 |
|--------------|-----------|---------------------------------|
|    0xFF      | Exclusive | Change to Uninitialised Request |
|    0x00      | State     | Change to Setup Request         |
|    0x01      | Changes   | Change to Normal Mode           |
| 0x02 to 0x07 |           | Reserved                        |
|--------------|-----------| ------------------------------- |
|    0x0C      | Service   | Turn on Heartbeat               |
|    0x0D      | Usage     | Turn off Heartbeat              |

Service Usage Command Code Turn on Heartbeat will cause a OPC_HEARTB(0xAB) to be sent every
5000mS.  Turn of Heartbeat will stop the transmission of OPC_HEARTB.

### Request Service Discovery - OPC_RQSD(0x78)

If the Service Index is zero, then the module will send OPC_SD(0xAC) with the number of 
services supported.  It will then send an OPC_SD for each of those services.

If the Service Index is greater than zero, then a single OPC_ESD(0xE7) will be sent relating
to that specific service.

### Request Diagnostic Data - OPC_RDGN(0x87)

If the Service Index is zero, then the module will send OPC_DGN(0xAC) with the number of 
services supported.  It will then send an OPC_DGN for each of those services.

If the Service Index is greater than zero, then a single OPC_DGN(0xAC) will be sent relating
to that specific service.

This OP-code is only available when using the diagnostics enabled service
class ```MinimumNodeServiceWithDiagnostics```.
