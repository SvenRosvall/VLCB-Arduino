# TODO List

This list contains minor code improvements that are not user facing.

User facing bugs and improvements are managed as
[GitHub Issues](https://github.com/SvenRosvall/VLCB-Arduino/issues).

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
Also, only run a task once every 1ms.

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
