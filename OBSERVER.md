# OBSERVER DESIGN FOR TIVIDY

- Purpose
	- The best Observer design for TIVIDY, not a patch of what exists
	- Every UML class diagram change, written so it can be drawn by hand
	- What the rest of the team must change to match
- No code in this file, only structure, rules and comment text

---
# VERDICT ON THE CURRENT DESIGN

- Right
	- Subject keeps a list of abstract observers, three concrete observers exist
	- Observers are notified after the change, the instance is the only subject
- Wrong
	- `update(instance)` tells an observer that something changed, never what
		- "Item assigned" and "workflow waiting" change no state, so state name alone cannot tell them apart
		- HistoryObserver cannot log "InProgress to Escalated" without remembering the previous state itself
	- `WorkflowInstance` has no getters, so the pull model has nothing to pull
	- Nothing says who the recipient of a notification is
		- The recipient is chosen at runtime by the strategy, a constructor argument cannot hold it
	- `WorkflowInstance` is already State Context, Memento Originator and Subject
		- A fourth responsibility (managing observers) makes it hard to test
	- Failure handling exists only in a sequence diagram
		- Step 26 has NotificationObserver writing to WorkflowHistory, but WorkflowHistory stores mementos
	- Dependency problems
		- WorkflowObserver includes WorkflowInstance and WorkflowInstance includes WorkflowObserver
		- Observers depend on the whole instance when they only need facts about one event

---
# RECOMMENDED DESIGN

- Push an immutable event object
	- `update(const WorkflowEvent& event)`
	- The event carries everything an observer needs, as values
	- Observers no longer depend on WorkflowInstance at all
- Extract the Subject
	- New `WorkflowSubject` holds the list, attach, detach and notify
	- `WorkflowInstance` inherits from it
	- GoF participants map one to one: Subject, ConcreteSubject, Observer, ConcreteObserver
	- Observer tests run against a tiny test subject, without building a workflow
- One failure channel: exceptions
	- An observer that cannot do its job throws
	- The subject catches, continues with the other observers, and returns the failure messages
	- The instance publishes each failure as an OBSERVER_FAILED event, so the audit trail shows it
- One place publishes
	- Only `WorkflowInstance` creates events and notifies
	- Commands, states and the facade never notify
- Observers are created and deleted by the facade
	- Instance never deletes an observer
- Why this is the best fit here
	- One thread, small data, so copying a small event is cheap
	- The audit trail is a list of events, so HistoryObserver stores copies
	- The notification recipient travels inside the event
	- Matches the push of data your sequence diagrams already show

---
# ALTERNATIVES REJECTED

- Pull with state getters only
	- Cannot say what happened, observers can see a state that has already moved on
- Pull plus a "last event" field on the instance
	- Mutable shared field, overwritten if an observer causes another event
	- Needs the instance to stay alive while observers read it
- Subscribing to event types
	- `attach(observer, type)` adds bookkeeping, filtering inside `update` is simpler with three observers
- Return values from `update`
	- The subject would depend on observer results, which defeats the pattern
- Mediator instead of Observer
	- Nothing here is many to many, see the earlier discussion
- Smart pointers for observers
	- Not used in this project, manual ownership rules below

---
# UML CHANGES: NEW ELEMENTS

- NEW enumeration `EventType`
	- Stereotype: enumeration
	- Values
		- STATE_CHANGED
		- ITEM_COMPLETED
		- ITEM_ASSIGNED
		- WORKFLOW_WAITING
		- ESCALATED
		- ESCALATION_UNHANDLED
		- UNDONE
		- OBSERVER_FAILED
- NEW class `WorkflowEvent`
	- Immutable value object: set once through the constructor, no setters
	- Attributes, all private
		- `-type : EventType`
		- `-sequence : int`
		- `-instanceName : string`
		- `-fromState : string`
		- `-toState : string`
		- `-itemName : string`
		- `-participantName : string`
		- `-recipient : Participant*`
		- `-message : string`
		- `-itemsDone : int`
		- `-itemsTotal : int`
	- Operations
		- One getter per attribute, `+getType() : EventType`, `+getSequence() : int` and so on
		- `+describe() : string` returns a one line readable text for the TUI, GUI and logs
	- Field meaning
		- `sequence` counts events per instance from 1, set by the instance
		- `fromState` is filled only for STATE_CHANGED, ESCALATED and UNDONE, otherwise empty
		- `toState` is always the instance state after the event
		- `itemName` is empty when the event is not about a work item
		- `participantName` is the person involved, empty if none
		- `recipient` is who must be told, may be null, never owned
		- `message` is the reason or extra text (escalation reason, failure text)
		- `itemsDone` and `itemsTotal` are the progress snapshot at the time of the event
- NEW class `WorkflowSubject`
	- Subject in the Observer pattern
	- Attributes
		- `-observers : vector<WorkflowObserver*>`
	- Operations
		- `+attach(observer : WorkflowObserver*) : bool` true if added, false if null or already attached
		- `+detach(observer : WorkflowObserver*) : bool` true if removed, false if it was not attached
		- `+getObserverCount() : int`
		- `#notifyObservers(event : WorkflowEvent) : vector<string>` returns one message for each observer that threw, empty if all succeeded
	- Notes on the class
		- Virtual destructor
		- Copying disabled, state this in a UML note
		- Does not delete observers
	- Notify is protected so only the instance can publish

---
# UML CHANGES: EXISTING CLASSES

- `WorkflowObserver`
	- CHANGE `update(instance : WorkflowInstance*) : void` to `update(event : WorkflowEvent) : void`
	- Keep the return type void, it is correct
	- Keep the virtual destructor
- `HistoryObserver`
	- ADD attribute `-entries : vector<WorkflowEvent>`
	- ADD `+getEntries() : vector<WorkflowEvent>` returned as a read only reference
	- ADD `+getEntryCount() : int`
	- CHANGE `update(event : WorkflowEvent) : void`, appends a copy of every event it receives
	- Behaviour: logs all event types, including OBSERVER_FAILED
- `ProgressObserver`
	- ADD attributes
		- `-stateName : string`
		- `-itemsDone : int`
		- `-itemsTotal : int`
	- ADD operations
		- `+getStateName() : string`
		- `+getItemsDone() : int`
		- `+getItemsTotal() : int`
		- `+getPercentComplete() : int` returns 0 when total is 0
	- CHANGE `update(event : WorkflowEvent) : void`, overwrites the snapshot from the event, never prints
- `NotificationObserver`
	- ADD attribute `-service : NotificationService*`
	- CHANGE the dashed dependency to `NotificationService` into a solid association, multiplicity 1, non owning
	- CHANGE `update(event : WorkflowEvent) : void`
	- Behaviour
		- Acts only on ITEM_ASSIGNED, ESCALATED, and STATE_CHANGED into Resolved or Closed
		- Ignores WORKFLOW_WAITING, UNDONE, ESCALATION_UNHANDLED and OBSERVER_FAILED
		- Skips the event when `recipient` is null
		- Lets any failure from the service propagate as an exception
- `NotificationService`
	- Keep `sendNotification(recipient : Participant*, message : string) : void`
	- ADD virtual destructor, it is an interface
	- Failure is signalled by an exception, state this in a UML note
- `EmailAdapter`
	- Lets a failure from `ExternalEmailService` propagate, with context text added
	- No operation changes
- `WorkflowInstance`
	- Inherits from `WorkflowSubject`
	- REMOVE `-observers`, `+attach`, `+detach`, `+notifyObservers`
	- ADD attribute `-nextSequence : int`
	- ADD getters
		- `+getName() : string`
		- `+getStateName() : string`
	- ADD `-publish(type : EventType, item : string, participant : Participant*, message : string) : void`
		- Builds the event with the next sequence number and the progress snapshot
		- Calls `notifyObservers`
		- Publishes one OBSERVER_FAILED event for each failure returned
		- Failures raised while delivering an OBSERVER_FAILED event are ignored, so there is no loop
	- ADD `+processCompletion(item : WorkflowComponent*) : void`, present in the sequence diagram
	- CHANGE `setState` so it publishes STATE_CHANGED after the change
	- CHANGE `escalate()` to take a reason, `escalate(reason : string)`, the sequence diagram already does this
	- CHANGE `restore` so it publishes UNDONE, it must not touch the observer list
- `WorkflowComponent` and its subclasses
	- ADD `+countItems() : int` and `+countCompleted() : int`
	- WorkItem returns 1 and 0 or 1
	- WorkflowStage and SubProcess add up their children
	- WorkItemDecorator forwards to the wrapped component
	- Needed for the progress snapshot
- `WorkItem`
	- REMOVE the association line to `WorkflowObserver`

---
# UML CHANGES: LINES

- REMOVE
	- Aggregation from `WorkflowInstance` to `WorkflowObserver`
	- Association from `WorkItem` to `WorkflowObserver`
	- Dependency from `WorkflowObserver` to `WorkflowInstance`
- ADD
	- Generalization from `WorkflowInstance` to `WorkflowSubject`
	- Aggregation (hollow diamond) from `WorkflowSubject` to `WorkflowObserver`, multiplicity 0..*
	- Dependency from `WorkflowObserver` to `WorkflowEvent`
	- Dependency from `WorkflowSubject` to `WorkflowEvent`
	- Dependency from `WorkflowInstance` to `WorkflowEvent`, it creates them
	- Composition (filled diamond) from `HistoryObserver` to `WorkflowEvent`, multiplicity 0..*
	- Association from `WorkflowEvent` to `EventType`
	- Association from `WorkflowEvent` to `Participant`, multiplicity 0..1, non owning
	- Solid association from `NotificationObserver` to `NotificationService`, multiplicity 1
- KEEP
	- Generalization from the three concrete observers to `WorkflowObserver`
	- `EmailAdapter` inherits `NotificationService` and uses `ExternalEmailService`

---
# CONTRACT: RULES THE CODE MUST FOLLOW

- R1: one publisher
	- Only `WorkflowInstance::publish` notifies
	- Commands, states and the facade call instance operations, never `notifyObservers`
- R2: after the change
	- Publish only once the change is complete, observers see the new state
	- One change publishes one event, never two for the same thing
- R3: order
	- Observers are notified in attach order
	- Attach order is HistoryObserver, ProgressObserver, NotificationObserver
	- History first means the event is logged before anything can fail
- R4: iteration is safe
	- Iterate over a copy of the list
	- Before calling each observer, check it is still attached
	- An observer may detach itself, it must not detach or delete another observer or the instance
- R5: failure
	- An observer that cannot do its job throws
	- The subject catches, goes on with the rest, and returns the failure messages
	- The state change is never undone
- R6: ownership
	- The facade creates observers, attaches them, and when an instance ends detaches them, deletes them, then deletes the instance
	- `WorkflowSubject` and `WorkflowInstance` never delete an observer
	- Participants are deleted after the observers, because events hold a non owning recipient pointer
- R7: events are values
	- No setters, no pointers owned
	- HistoryObserver keeps copies and never dereferences `recipient`, it uses `participantName`
- R8: duplicates and nulls
	- Attaching the same observer twice returns false and changes nothing
	- Detaching an observer that is not attached returns false and changes nothing
	- Attaching null returns false
- R9: single thread
	- All calls come from one thread, state this in the Doxygen text
- R10: one instance, one set
	- Every instance has its own HistoryObserver, ProgressObserver and NotificationObserver
	- No observer is shared between instances

---
# WHO PUBLISHES WHAT

- `setState`
	- STATE_CHANGED with from and to state
- `processCompletion`, completion part
	- ITEM_COMPLETED with the item
- `processCompletion`, dependencies satisfied
	- ITEM_ASSIGNED with the next item and the selected participant as recipient
- `processCompletion`, dependencies not satisfied
	- WORKFLOW_WAITING, history and progress only
- Escalation, handler found
	- ESCALATED with from and to state, the handler as recipient, the reason as message
- Escalation, no handler found
	- ESCALATION_UNHANDLED, needs the fail handler in the Chain of Responsibility
- `restore`
	- UNDONE with from and to state
- A failure returned by `notifyObservers`
	- OBSERVER_FAILED with the failure text as message
- Who sees which
	- HistoryObserver: all
	- ProgressObserver: all, it only copies the snapshot
	- NotificationObserver: ITEM_ASSIGNED, ESCALATED, final state changes

---
# CHANGES FOR THE OTHER TEAM MEMBERS

- Core workflow owner
	- Make `WorkflowInstance` inherit `WorkflowSubject`
	- Implement `publish` and call it from the places above
	- Add `getName`, `getStateName`, `processCompletion`, and the reason parameter on `escalate`
- Work structure owner
	- Add `countItems` and `countCompleted` to the Composite and Decorator classes
- Routing owner
	- Add the fail handler so ESCALATION_UNHANDLED can be raised
	- The strategy result becomes the `recipient` for ITEM_ASSIGNED
- Actions and history owner
	- Commands never notify
	- The memento caretaker stays `WorkflowHistory`, it is not the audit trail
- Integration owner
	- Facade creates, attaches, detaches and deletes the observers
	- Facade gives the UI read access to HistoryObserver and ProgressObserver
	- ExternalEmailService throws when sending fails, EmailAdapter lets it pass
- Sequence diagram: completing a work item
	- Add lifelines for HistoryObserver and ProgressObserver
	- Replace `notify(NextItem assigned)` with a self message `publish` followed by `update(event)` to each observer
	- Remove the return arrows "notification recorded" and "event recorded", `update` returns nothing
	- Show ITEM_COMPLETED after `complete()`
	- Show WORKFLOW_WAITING as a history and progress event, with no email
- Sequence diagram: escalating
	- Steps 19 and 20 become `publish` then `update(event)` to HistoryObserver, ProgressObserver and NotificationObserver
	- Step 19 is addressed to WorkflowHistory, it must be HistoryObserver
	- Step 26 `record(notification failed)` is wrong, show an exception coming back from `update`, then the instance publishing OBSERVER_FAILED to the observers
	- Step 6 `storeMemento` to WorkflowHistory is correct, keep it
- State diagram and the Escalated state decision
	- Outside Observer, tracked in the changes file

---
# FIXES FOR THE FILES ALREADY WRITTEN

- All three `.cpp` files throw a string literal
	- `throw "Not yet implemented"` throws a `const char*`
	- A catch for `std::exception` will not catch it, and tests cannot check it
	- Throw `std::logic_error` with the class and function name instead
- `WorkflowObserver.h`
	- Remove `#include "WorkflowInstance.h"` and the forward declaration, include `WorkflowEvent.h` only
	- Change the destructor comment from `/* */` to `/** */`, Doxygen ignores the first form
	- Rewrite the `update` comment, see below
- `ProgressObserver.h`
	- Remove `#include "WorkflowInstance.h"`
- `HistoryObserver.h`
	- Remove the commented out include
- `NotificationService.h`
	- Add a virtual destructor, deleting through the interface is undefined behaviour without it
	- Forward declare `Participant` instead of including it
	- Take the message as `const std::string&`
- Include style
	- Pick `"engine/X.h"` or `"X.h"` and keep it everywhere

---
# DOXYGEN TEXT FOR WorkflowObserver

- Class comment

```cpp
/**
 * @brief Observer pattern: abstract Observer.
 *
 * Implemented by anything that reacts to something happening in a
 * WorkflowInstance, such as logging, progress display or notifications.
 * The subject knows only this interface, never the concrete observers.
 *
 * @par Push model
 * update() receives a WorkflowEvent that already holds everything the
 * observer needs. Observers never read the instance.
 *
 * @par Ownership
 * WorkflowSubject keeps a list of observers but does not own them. The
 * creator deletes an observer and must detach it first.
 *
 * @par Concrete observers
 * NotificationObserver, HistoryObserver and ProgressObserver.
 *
 * @see WorkflowSubject, WorkflowEvent
 */
```

- update comment

```cpp
/**
 * @brief Called after the instance has changed.
 *
 * @param event What happened. Safe to copy. Do not keep pointers from it.
 *
 * @note May detach itself. Must not detach or delete other observers.
 * @note Throw if the observer cannot do its job. The subject records the
 *       failure and notifies the remaining observers. The change stays.
 */
```

---
# TESTS FOR THIS DESIGN

- WorkflowSubject, using a small test subject that exposes `notifyObservers`
	- Attach and notify calls the observer once
	- Observers are called in attach order
	- Detached observer is not called
	- Duplicate attach and unknown detach return false and change nothing
	- Observer that detaches itself during `update` does not break the loop
	- Observer that throws does not stop the others, its message is returned
	- Destroying the subject does not delete the observers
- WorkflowEvent
	- Getters return what the constructor was given
	- `describe` mentions the type and item
- HistoryObserver
	- Entries appear in order, one per event
	- Entries are copies and survive after the recipient is gone
	- OBSERVER_FAILED events are logged
- ProgressObserver
	- Snapshot is replaced by each event
	- Percentage is 0 when total is 0
- NotificationObserver, using a fake service that records calls and can be told to throw
	- Sends the right recipient and message for ITEM_ASSIGNED and ESCALATED
	- Sends nothing for WORKFLOW_WAITING
	- Sends nothing when the recipient is null
	- Failure from the service propagates
- WorkflowInstance with all three observers
	- `setState` publishes one STATE_CHANGED with the correct from and to
	- Sequence numbers rise by one for each event
	- A failing observer produces an OBSERVER_FAILED entry in the history
	- Two instances do not receive each other's events
	- `restore` does not remove observers
- Run the sanitizer build, observers and pointers are where it finds problems

---
# REPORT TEXT TO UPDATE

- Subject: `WorkflowSubject`, ConcreteSubject: `WorkflowInstance`
- Observer: `WorkflowObserver`, ConcreteObservers: the three
- Supporting class: `WorkflowEvent`, an immutable event
- Model: push, with the event as payload
- Failure behaviour: exception per observer, collected and recorded as an event
- Ownership: facade owns the observers, the subject does not
- Remove all mention of WorkItem as a subject
- Remove EmailAdapter as an observer
- Remove "check whether another work item can begin" as an observer reaction