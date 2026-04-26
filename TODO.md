# TODO List

## Introduce TimedResponse
The Action queue has a limited size of 30 which is enough to handle
all responses for RQNPN.
If the node responds with more messages than the queue size then the 
first messages will be dropped.
This happened to Nick Locke where he had 59 events in response to NERD.
The first 28 events were dropped.

Ian Hogg implemented a TimedResponse algorithm that produces response 
events in a slower pace to allow for the CAN transport to work off the Action queue.
This also has the advantage that the pending actions don't use any memory
as they have not been created yet.

General algorithm:
1. Response producer registers work to do with a callback function / object.
2. At each process() call, check for time interval. (optional)
3. Call the callback with a sequence number. Possibly also other information.
4. The callback does its work for that sequence number. Return a state to indicate
   task the job is complete.

The sequence number can indicate which response to generate, such as parameter
number or event at the index given by the sequence number.

If an object is used instead of a callback function then it can hold
the sequence number, or whatever it needs to keep track of work.
There is a case where all diagnostics for all services are returned.
Here the callback object can contain the service index and the diagnostic index.
It can also contain a timer so that if it doesn't get enough time
to generate its messages within a time, where the requestor gives up,
the generation of sent messages can be abandoned.

This will be part of `Controller` where the action queue exists.

Ian's implementation can only handle one such task at a time. 
This may be limiting. Allow for a small queue of such tasks.
Typically, there won't be a need for more than one such task as
the requestor (MMC or FCU) will wait for responses before sending the
next request.
Responses to Start-of-Day may get intermingled with MMC/FCU requests.

Throttle running of these tasks if the Action queue is getting full.

Could this be implemented as a service?
This would then be run from the general process() loop.
This would separate the task management from the `Controller` class which is 
big enough already.
It may be tricky for other services to get hold of this service to place a task.

## Keep Node data in Controller
NodeNumber etc are split across ```Controller``` and ```Configuration```. 
Keep all access to these in ```Controller```. 
The ```Controller``` shall delegate to ```Configuration``` for persisting data.

Controller today has
* _params
* _mname

Configuration today has
* currentMode (FLiM/SLiM)
* nodeNum

Need to change the notion of SLiM/FLiM to VLCB modes.
Should be part of the MNS service.


## Updates to the Storage interface
The begin method should take a size parameter. 
See calculation of this size in EepromInternalStorage.cpp. 
This calculation should sit in Controller. 
The various storage implementations may choose to ignore this size value.

## Provide access to persistent storage for user code
The Storage interface provides a simple API to persistent storage regardless of
type of storage. 
The user code should be able to reserve a chunk of bytes in this space.

## Split SerialUserInterface
The SerialUserInterface class contains everything a developer could need.
This adds code bloat. 
Split this class in two: 
  1. do the same things as the push-button does, i.e. initiate CAN enumeration, 
     initiate setup mode.
  1. all the other stuff a developer would want, such as transport statistics and
     resetting the module.

## Introduce InternalDiagnosticsService
If SerialUserInterface is not included in the service list then there is no
way to check free memory.
A dedicated service could provide free memory amount as a diagnostic value.

What internal information would be useful for this service?
* Free memory. (as listed above)
* Monitor the action queue. Current use, high watermark. (See CanService buffer use for inspiration.)
* Count of generated actions. 
* TimedResponse data such as number of tasks in queue, total number of tasks created. 

Is this a service that should be included in the VLCB specifications?
Or should it be treated as a user defined service?

## Move CAN2515 to its own package/repo
Now that we are getting more CAN transport implementations it is time to move CAN2515 
away into its own library. 
This breaks the dependency on ACAN2515 which should not be required for other CAN transports.

## Ideas for reducing memory usage
### Make CanFrame polymorphic
Use virtual functions to get each field.
Each CAN transport implementation must implement CAN Frame functions.

This should reduce the amount of copying between CanFrame and the datastructure used 
within the CAN transport implementation.

However, the added coding and memory used for virtual functions may negate the benefits 
of a polymorphic CanFrame class.

The result was that there wasn't much benefit here.

Another approach is to use a template CanFrame.
Each CAN transport implements an instantiation of CanFrame where each getter/setter
accesses the elements of the datastructure used for that transport.
This means there is no conversion between the specific CAN datastructure
and the VLCB::CanFrame. 

This proved to reduce memory and code size significantly.
But the code is harder to understand.

## Throttle action queue
If the CAN service cannot deliver outgoing CAN frames fast enough then the
buffers will spill over.
How can we throttle the generation of outgoing messages?

The tricky bit is that the action queue is general for all services
and cannot just be throttled on one particular service.

A few thoughts:
1. Don't accept the action. This means the action will be left in the action
queue until next process() loop. All services will now see this action again.
This may affect `EventConsumerService` if the COE flag is set.
This blocks any other actions such as activity indications or incoming messages.
2. Accept the action but put it back in the action queue. This lets other
actions through. But other services will see this outgoing message action again.
3. Add a flag to the Action Queue that action consumers can raise if they cannot
keep up. This flag tells action producers that they cannot put more entries
into the action queue.
   1. Need some mechanism to allow more than one service to block action production.
   Use a [Counting Semaphore](https://en.wikipedia.org/wiki/Semaphore_(programming))
   where each busy service increases the semaphore value. The services decrease
   the value when they are not busy any more. Action producers only produce actions
   when the semaphore value is 0.
      1. There might not be more services than `CanService` that needs to
      throttle action creations. May need to prepare for bridging applications
      such as CANCAN.
   2. May need to flag which action type should be blocked. Probably not 
   necessary as this is only a problem with sending messages, i.e. `ACT_MESSAGE_OUT`.
   3. Action throttling is only a problem for multiple responses, i.e. things
   generated by [TimedResponse](#introduce-timedresponse) described above.
   Single messages should still be allowed to be sent out to avoid complexity
   of queueing actions for any piece of code that creates messages.
   Of course the action consumers must be able to receive these single messges,
   which means that CAN transport must have enough buffer space for those messags.

## Documentation

### Split documentation based on audience
Need to clarify which documents are intended for which audience.
There are a few distinct kinds of audiences:
  1. Developers of VLCB-Arduino library.
  2. Sketch creators that use the VLCB-Arduino library.
  3. Users of created sketches. These should probably be catered for by the sketch authors.

### How-to guide for Sketch Authors
Describe how to write a sketch with VLCB setup. 
How to create service objects and the controller object.
How to configure any parameters and service specifics.

