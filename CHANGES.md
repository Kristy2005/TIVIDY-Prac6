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

- DONE: implementation
	- WorkflowInstance given a real constructor, destructor, getState(), setState(), attach(), detach(), notifyObservers() — all five lifecycle operations (activate, startProcessing, resolve, close) now delegate to the current WorkflowState instead of throwing "Not yet implemented"
	- InProgressState::close() fixed to transition to ClosedState. The header already documented close() as allowed from In Progress, but the .cpp threw "Cannot close an in-progress workflow instance" — the two now agree
	- Each transition notifies observers exactly once. setState() is the single place that calls notifyObservers(); the duplicate manual notifyObservers() call that used to sit inside SubmittedState::activate(), ActiveState::startProcessing(), ActiveState::close() etc. (marked with a `//TODO - DISCUSS CHANGES ON HOW NOTIFY OBSERVERS WORKS` comment) has been removed from every state .cpp
- DECIDE (resolved): state ownership
	- Two designs were tried. Owned states: WorkflowInstance allocates a state with `new` in its constructor, setState() deletes the old state and takes ownership of the new one, and a Prototype clone() was added to WorkflowState and all five concrete states so WorkflowMemento could hold its own independent copy. Shared states: one `static` object per concrete state, constructed once and never deleted; WorkflowInstance and WorkflowMemento both hold non-owning pointers to the same long-lived object
	- Team decided: shared states. Reasons — it matches WorkflowState.h's original documented design; the five state classes hold no member data, so cloning one is just `return new ActiveState();` with no real benefit; owned states meant setState() deleted the very object whose own member function (e.g. activate()) was still executing on the call stack, legal only because nothing after that call touched `this`, which is fragile to maintain; and Prototype was never in the Practical 6 UML, so adding clone() would have meant an undocumented pattern in the code with no diagram to match
	- clone() removed from WorkflowState.h and every concrete state header. Every `new XState()` inside a transition method was replaced with a function-local `static XState x; instance->setState(&x);`
- CHANGE: memory-safety bug introduced mid-revert
	- After reverting to shared states, WorkflowInstance::setState() still contained `delete old;` left over from the owned-state design. Since `old` now always points at a `static` local (never allocated with `new`), this was an invalid free — undefined behaviour, and a likely crash the first time any instance transitioned. Removed; shared states are never deleted, by any code, anywhere
	- createMemento()/restore() comments ("the memento gets its own copy" / "deletes the old state and notifies") were left over from the owned-state version and no longer described what the code did. Reworded to say the memento stores the same shared pointer, and that restoring never deletes anything
- NOT DONE: needs the team
	- escalate(): InProgressState.h's doc comment still describes escalate() as a transition belonging to the State pattern ("moves the instance to EscalatedState"), but there is no EscalatedState class and WorkflowState declares no escalate() operation. The real escalate() lives entirely on WorkflowInstance: it checks `dynamic_cast<InProgressState*>(currentState)` and, if that succeeds, forwards to `definition->handleEscalation()`; the case stays In Progress. Team needs to decide whether escalation becomes a proper fifth WorkflowState operation (adds an EscalatedState, or keeps the instance In Progress) or whether InProgressState.h's comment should just be corrected to describe the dynamic_cast approach actually in use
	- `final` was recommended for ResolvedState and ClosedState (per WorkflowState.h's own note that they "reject every transition") but has not been added to either class declaration yet
	- Misleading error message wording, flagged early and still unresolved:
		- SubmittedState::resolve() throws "Resolution not yet implemented" — reads as a stub, but the behaviour (reject resolve from Submitted) is intentional and permanent, not a TODO
		- ActiveState::resolve() throws "Cannot resolve an inactive workflow instance" — the instance is Active, not inactive, when this fires; the wording should explain that work hasn't started yet, not that the instance is inactive
	- StateTests.cpp's makeInstanceIn() helper still does `instance->setState(new S())` to force a test instance into a given starting state. Under the shared-state model this leaks one object per test case (harmless, test-only, confined to that file) unless changed to `static S s; instance->setState(&s);`

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

- DONE: EmailAdapter null-handling contract decided and implemented
	- EmailAdapter::sendNotification() threw std::invalid_argument for a null recipient and std::logic_error for a missing service, which contradicted EmailAdapter.h's own doc comment at the time ("Does nothing if the recipient or the service is null")
	- Team decided: Option B — keep the throwing behaviour, fix the documentation instead of the code. A missing recipient or unconfigured service is a caller error; surfacing it immediately is more useful than a silent no-op that could hide a notification that never went out
	- EmailAdapter.h rewritten: doc comment now states the real contract (`@throws std::invalid_argument` for a null recipient, `@throws std::logic_error` for a missing service), with a `@par Preconditions` paragraph explaining why these are enforced rather than silently skipped
	- Tests updated to match: the two cases that previously expected a silent no-op (`CHECK_FALSE(run.threw)`) now assert `CHECK_THROWS_AS(..., std::logic_error)` / `CHECK_THROWS_AS(..., std::invalid_argument)`
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
- DONE: ADD items below implemented
	- `WorkflowState* getState() const;` and `void setState(WorkflowState* state);` added to WorkflowMemento
	- These store the same shared WorkflowState pointer the instance holds, not a copy — consistent with the STATE section's decision to keep states as shared, stateless singletons. No clone() was needed on WorkflowMemento's side either, for the same reason
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