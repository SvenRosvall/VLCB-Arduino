# TODO List

## Multiple services
Introduce ```initializer_list``` class so that the controller
can be set up like this:
```
VLCB::Controller controller(&modConfig, &ledUI, {&mnsService, &evConsumeSvc};
```

## Keep Node data in Controller
NodeNumber etc are split across Controller and Configuration. 
Keep all access to these in Controller. 
The Controller shall delegate to Configuration for persisting data.

Controller today has
* _params
* _mname
* bModeChanging
* timeOutTimer (used to decide to stop changing mode)

Configuration today has
* currentMode (FLiM/SLiM)
* nodeNum

Need to change the notion of SLiM/FLiM to VLCB modes.
Should be part of the MNS service.

## Keep CAN data in one place
Same goal as for node data above to keep things in the Controller. 
It may delegate to Configuration for persisting data.

Controller:
* enum_responses
* bCANenum
* CANenumTime

Configuration:
* CANID

Enumeration is controlled by entering FLiM mode and received OP codes.
This might be defined in a CAN service. 

## Update LedUserInterface class
The Minimum Node Specification describes a few more states that the LED's need to indicate. 
Review this spec again and update the user interface. 
Also need to update the interface class and add support for these states in the Controller class.
