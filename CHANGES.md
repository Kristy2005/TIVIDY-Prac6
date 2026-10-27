# FINAL DOCUMENT CHANGES

- Purpose
	- Lists what must change in the Practical 6 pattern text so it matches the current UML class diagram and the code
- How to read
	- CHANGE: text exists but is wrong or out of date
	- ADD: text is missing
	- REMOVE: text must go
	- DECIDE: the team must pick an option before coding or editing

---
---
# OBSERVER: DOCUMENTATION AND SIMPLE FIXES

- DONE: documentation
	- Doxygen comments on WorkflowObserver, HistoryObserver, ProgressObserver, NotificationObserver
	- Doxygen comments on NotificationService, EmailAdapter, ExternalEmailService
	- Doxygen comments on the observer operations of WorkflowInstance
	- Comments describe the current pull model and mark unfinished behaviour with `@todo` and `@warning`
- DONE: header fixes
	- WorkflowObserver.h: namespace closed before `#endif`, stray brace after the guard removed
	- WorkflowObserver.h: destructor comment changed to `/** */`, extra `;` removed
	- NotificationObserver.h: class comment moved inside the namespace, directly above the class
	- NotificationService.h: virtual destructor added
	- EmailAdapter.h: `override` added, default constructor sets `service` to null
	- HistoryObserver.h: commented out include removed
- CHANGE: comments that promised behaviour that does not exist
	- "Instance records the failure and notifies the remaining observers" became a `@todo`
	- "Holds the recipient and the service" became a `@warning`, the class has neither
- NOT DONE: needs the team
	- Push or pull, and what an observer is told about a change
	- Failure handling when an observer throws
	- Extracting a WorkflowSubject
	- NotificationObserver recipient and service
	- HistoryObserver as an audit trail of events, not mementos
	- ProgressObserver reporting the state and the amount of work done
	- Participant email address for EmailAdapter
	- `sendNotification` taking the message by const reference
---
# BUILDER

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# STATE

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# STRATEGY

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# CHAIN OF RESPONSIBILITY

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# COMMAND

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# COMPOSITE

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# ADAPTER

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# FACADE

- CHANGE:
- ADD:
- REMOVE:
- DECIDE:

---
# MEMENTO

- CHANGE: Nothing for now
- ADD: 
```c++
WorkflowState* getState()const;
void setState(WorkflowState* state);
```
- REMOVE: Nothing for now 
- DECIDE: 
	- Whether we should make it a flyweight such that when a WorkflowInstance member variables are altered 
	- Observers are still in tact 
	- WorkflowDefinition is still in tact 

---
# DECORATOR

- CHANGE:
- ADD:
- REMOVE:
- DECIDE: