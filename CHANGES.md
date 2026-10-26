# FINAL DOCUMENT CHANGES

- Purpose
	- Lists what must change in the Practical 6 pattern text so it matches the current UML class diagram and the code
- How to read
	- CHANGE: text exists but is wrong or out of date
	- ADD: text is missing
	- REMOVE: text must go
	- DECIDE: the team must pick an option before coding or editing

---
# OBSERVER

- CHANGE: subject
	- "WorkflowInstance or WorkItem" becomes "WorkflowInstance" everywhere
	- Remove the stray UML line from WorkItem to WorkflowObserver
- REMOVE: "Check whether another work item can begin" as an observer reaction
	- This is engine logic done by WorkflowInstance through the definition
	- If it were an observer, the workflow would stop when nobody attached it
- CHANGE: EmailAdapter is not an observer
	- Chain is NotificationObserver, NotificationService, EmailAdapter, ExternalEmailService
- CHANGE: "Command execution also notifies observers"
	- The instance notifies, from one place
	- Otherwise observers hear about one event twice
- ADD: push or pull
	- `update(instance)` is the pull model
	- Observers read what they need from the instance
- ADD: ownership
	- The instance keeps the list, it does not own the observers (aggregation)
- ADD: failure behaviour
	- A failing observer is recorded
	- Remaining observers still run
	- The state change stays
- ADD: HistoryObserver and WorkflowHistory are different
	- HistoryObserver logs events for the audit trail
	- WorkflowHistory stores mementos
- ADD: getters the observers need
	- State name
	- Access to the definition
- ADD: NotificationObserver needs a recipient and a NotificationService
	- Show as an association, not only a dashed line
- DECIDE: item-level events
	- The problem text talks about "work item completed", but states only cover the whole instance
	- Option A: add instance operations for item assigned and item completed that call `notifyObservers`
	- Option B: limit Observer to state changes only

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