# Release Notes for VLCB-Arduino library

Releases are listed on GitHub: https://github.com/SvenRosvall/VLCB-Arduino/releases

# 2.1.0 - Example sketch clean-up
Update example sketches for better readability.

Add VLCB functions for convenient sending messages.

# 2.0.0 - Event Slot support
Major rework on how events are handled.

Support the concept of *event slots* where the event table consists
of a number of *slots*.
Each slot defines what to do for produced and consumed events.
See example sketch [VLCB_4in4out_slot](docs/Examples.md#vlcb_4in4out_slot).

To support slots the following changes were done:

* The treatment of the first event variable (EV#1) as specifying 
a produced event is moved from the library to user sketches.
It is now up to the sketch designer to decide the meaning of 
each event variable.

* There is no need to specify how many "producer" events the module uses.

* `EventProducerService` method `sendEvent()` used to send an event
that matched a given EV#1 value. 
It is replaced by the new method `sendEventAtIndex()` which takes an
event index as parameter. 
The event at this index will be sent.

* The `EventConsumerService` may call the `eventHandler` callback
multiple times, one for each slot that has an event with the same
event ID (node number / event number) as the incoming event.

* The example sketch `VLCB_4in4out` no longer uses the external Bounce2 
library for managing switches.
Code has been added to the sketch to create default events for module
switch changes. 
The module behaves as before.

* The example sketch `VLCB_1in1out` no longer creates default
events when the module switch changes.

# 1.1.0 - Minor fixes
* Improves how the green LED flashes on incoming messages to make messages for this node more distinguishable.
* Updates to how objects are set up. All of these changes are hidded by the VLCB convenience functions so if you use these VLCB convenience functions for setup then there will be no changes to your sketch.

# 1.0.0 - First distributed release
This release introduced helper functions in the ```VLCB``` namespace
and simplified module setup.