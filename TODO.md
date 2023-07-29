# TODO List

## Add helper sendMessage() 
There is some boiler code for every sendMessage(). 
Create helpers that fill in much code. 
Example use:
```
sendMessage5(OPC_NUMEV, NN, nEvents);
```

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
* currentMode
* nodeNum

Need to change the notion of SLiM/FLiM to VLCB modes.
Should be part of the MNS service.

## Keep CAN data in one place

Controller:
* enum_responses
* bCANenum
* CANenumTime

Configuration:
* CANID

Enumeration is controlled by entering FLiM mode and received OP codes.
This might be defined in a CAN service. 