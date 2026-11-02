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

- DONE: implementation
	- WorkflowBuilder (abstract) declares the full step interface: `addWorkItem`, `createWorkItem`, `addRole`, `createRole`, `addParticipant`, `createParticipant`, `addDependency`, `setAssignmentStrategy`, `setEscalationHandler`, `addPriority`, `beginSubProcess`, `endSubProcess`, `getResult`. All operations are pure virtual with Doxygen comments
	- ConcreteWorkflowBuilder implements WorkflowBuilder in two modes: creating mode (owns the definition) and configuring mode (edits an existing definition). TIVIDYFacade uses configuring mode via the private `builderFor()` helper
	- WorkflowDirector drives a WorkflowBuilder through two fixed recipes: `constructSequentialWorkflow` (each step depends on the previous) and `constructParallelWorkflow` (no dependencies). Both validate the step list before touching the builder
	- `WorkflowStep` struct added to WorkflowDirector.h — holds `name`, `role`, and `priority`
- CHANGE: UML shows only a plain `WorkflowBuilder` and `WorkflowDirector`; the code separates them into three classes (`WorkflowBuilder` interface, `ConcreteWorkflowBuilder`, `WorkflowDirector`) and adds `WorkflowStep`. The Facade also holds one `ConcreteWorkflowBuilder*` per definition in a `std::map`; none of this is in the diagram
- ADD: WorkflowDirector now exposes `constructParallelWorkflow` in addition to the sequential variant shown in the UML; the sequential recipe is accessible both through `constructSequentialWorkflow` (direct) and through `addStep` / `constructWorkflow` (accumulated steps)
- REMOVE: nothing removed relative to the UML
- DECIDE: Nothing open

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

- DONE: implementation
	- AssignmentStrategy (abstract) declares `assign(WorkItem*, vector<Participant*>)` as a pure virtual and provides a default no-op `assign(WorkflowInstance*, WorkItem*, vector<Participant*>)` for instance-aware callers. Both overloads are documented
	- RoleBasedAssignment: selects the first candidate whose role list contains the item's required role; returns nullptr when no role is set on the item or no candidate matches. Both the item-only and the instance-aware overloads are implemented and the second delegates to the first (instance is not used)
	- WorkloadBasedAssignment: selects the candidate with the fewest active assignments (lowest workload). Ties broken by candidate order. Both overloads implemented; the instance-aware one passes through to the item-only variant
	- WorkflowDefinition::selectAssignee() calls the configured strategy's instance-aware `assign()`. TIVIDYFacade::assignWork(instance, item, participants) invokes this and records the result with `instance->assignParticipant()`
- CHANGE: UML shows only `AssignmentStrategy` and one concrete strategy; the code provides two concrete strategies (`RoleBasedAssignment`, `WorkloadBasedAssignment`) and adds the instance-aware overload to both
- ADD: `WorkloadBasedAssignment` was not in the original UML diagram; it was added to support real workload-based routing
- REMOVE: nothing removed relative to the UML
- DECIDE: Nothing open

---
# CHAIN OF RESPONSIBILITY

- DONE: implementation
	- Handler (abstract): base with `successor` pointer, `setSuccessor()`, and a default `handleRequest()` that validates the instance and forwards to the successor when one exists; null requests throw `std::invalid_argument`
	- ApprovalHandler: validates the request and forwards it to the successor; used as the first link in the configured chain
	- EscalationHandler: validates and forwards; throws `std::logic_error` if no successor is configured (the chain has no terminal handler)
	- WorkHandler: validates and forwards; accepts with no action if no successor (i.e., it may be a standalone or terminal link)
	- FinalDecisionHandler: terminal handler — validates and accepts the request but currently records no decision and does not change workflow state
	- Chain is wired at runtime through `Handler::setSuccessor()`. WorkflowDefinition holds the head of the chain and calls `handleRequest()` on it from `handleEscalation()`
- CHANGE: UML shows only `Handler` and one concrete handler; the code provides four concrete handlers (`ApprovalHandler`, `EscalationHandler`, `WorkHandler`, `FinalDecisionHandler`). The class names and the number of participants differ from the original diagram
- ADD: `WorkHandler` and `FinalDecisionHandler` were not in the original UML diagram; they were added to demonstrate a complete three-link chain
- REMOVE: nothing removed relative to the UML
- NOT DONE: needs the team
	- FinalDecisionHandler currently accepts the request without recording a decision or changing the workflow state. If escalation is supposed to have visible effects (e.g. set a flag or change a state), the body of FinalDecisionHandler::handleRequest() must be filled in
	- EscalationHandler throws when there is no successor. If a chain of length one (only an ApprovalHandler) is a valid configuration, EscalationHandler's throwing behaviour is too strict; the policy should be decided and documented

---
# COMMAND

- DONE: implementation
	- Command (abstract): holds a non-owning `receiver` pointer and an owned `WorkflowMemento* previous`. `saveState()` deletes any prior memento and calls `receiver->createMemento()`. Destructor deletes `previous`. Pure virtual `execute()` and `undo()` documented
	- ApproveWorkCommand: `execute()` saves state, calls `receiver->activate()`; `undo()` calls `receiver->restore(previous)`
	- AssignWorkCommand: `execute()` saves state, calls `receiver->startProcessing()`. **Note**: despite the name this command does not assign a WorkItem — it starts processing on the instance
	- CompleteWorkCommand: `execute()` saves state, calls `receiver->resolve()`. **Note**: this calls raw `resolve()` with none of the "all items complete" check or the Active→InProgress guard in TIVIDYFacade::completeWork; do not use this command as a drop-in replacement for the Facade's completeWork
	- EscalateWorkCommand: `execute()` saves state, calls `receiver->escalate()`; `undo()` restores the snapshot. Matches the Facade's `escalateWork` semantics exactly
	- CancelWorkCommand: `execute()` saves state, calls `receiver->close()`. **Note**: closes the whole instance, not a single work item — it is functionally identical to CloseWorkflowCommand
	- CloseWorkflowCommand: `execute()` saves state, calls `receiver->close()`; `undo()` restores the snapshot. Matches the Facade's `closeWorkflow` semantics exactly
	- Invoker: non-owning `command` pointer, `setCommand()`, `executeCommand()` (throws if no command), `undoCommand()` (throws if no command). All documented
- DONE: Facade integration (this session)
	- TIVIDYFacade now owns one `Invoker`, one `EscalateWorkCommand`, and one `CloseWorkflowCommand` as private members
	- `escalateWork(instance)` sets the receiver on `escalateCmd`, configures the Invoker, and calls `invoker.executeCommand()` — the command saves a memento before escalating, enabling undo
	- `closeWorkflow(instance)` sets the receiver on `closeCmd`, configures the Invoker, and calls `invoker.executeCommand()` — same memento/undo path
	- `undoLastCommand()` added to TIVIDYFacade's public interface; it delegates to `invoker.undoCommand()`, restoring the instance to its pre-escalation or pre-closure state
	- `Invoker.h`, `EscalateWorkCommand.h`, and `CloseWorkflowCommand.h` added to the TIVIDYFacade.h includes
	- The UML's dependency arrow from TIVIDYFacade to Invoker is now accurate
- CHANGE: three commands have misleading names relative to their actual behaviour
	- AssignWorkCommand starts processing, not work assignment (see note above)
	- CompleteWorkCommand resolves the instance directly, bypassing Facade safeguards (see note above)
	- CancelWorkCommand closes the whole instance rather than cancelling a single item (see note above)
- ADD: `undoLastCommand()` on TIVIDYFacade — not in the original UML, added to expose the undo capability to callers. Only the last escalate or close is undoable; earlier operations are not tracked by the Invoker
- NOT DONE: needs the team
	- AssignWorkCommand, CompleteWorkCommand, and CancelWorkCommand are not wired through the Facade because their implemented behaviour does not match the Facade method with the same name. The team should either rename the commands to match what they actually do, or rewrite their `execute()` to match the Facade's validated logic so they can be safely integrated

---
# COMPOSITE

- DONE: implementation
	- WorkflowComponent (abstract): declares `execute()`, `isComplete()`, `collectWorkItems()`, and `clone()`. Component participant in the Composite pattern
	- WorkItem (Leaf): implements WorkflowComponent. Holds `name`, `completed`, `requiredRole`, and `priority`. `execute()` marks the item as started, `complete()` marks it done, `isComplete()` returns the flag
	- WorkflowStage (Composite): holds a vector of non-owning WorkflowComponent pointers. `add()`, `remove()`, and a `swap()` helper used by the builder when wrapping an item in a priority decorator. `execute()` calls execute on every child; `isComplete()` returns true when every non-cancelled child is complete; `collectWorkItems()` recurses into children
	- SubProcess (Composite): same interface as WorkflowStage, used for nested sub-processes opened with `beginSubProcess()` / `endSubProcess()` in the builder. Structurally identical to WorkflowStage but semantically distinct (a sub-process groups related steps and can be nested arbitrarily deep)
	- PriorityDecorator wraps a WorkflowComponent and stores a priority value. The builder's `addPriority()` replaces an item in the stage with its wrapped decorator so the stage traverses the decorator transparently. The decorator is a Decorator (see DECORATOR section) but participates in the Composite tree as a leaf-wrapping node
- CHANGE: UML shows only `WorkflowComponent`, `WorkItem`, and `WorkflowStage`; the code adds `SubProcess` as a second composite participant. The `swap()` helper on WorkflowStage and SubProcess is not in the UML but is needed by the builder
- ADD: `SubProcess` was not in the original UML diagram; it was added to support nested sub-processes in workflow definitions
- ADD: `collectWorkItems()` added to WorkflowComponent to allow traversal of the tree to gather all leaf WorkItems; not in the UML
- REMOVE: nothing removed relative to the UML
- DECIDE: Nothing open

---
# ADAPTER

- DONE: EmailAdapter null-handling contract decided and implemented
	- EmailAdapter::sendNotification() threw std::invalid_argument for a null recipient and std::logic_error for a missing service, which contradicted EmailAdapter.h's own doc comment at the time ("Does nothing if the recipient or the service is null")
	- Team decided: Option B — keep the throwing behaviour, fix the documentation instead of the code. A missing recipient or unconfigured service is a caller error; surfacing it immediately is more useful than a silent no-op that could hide a notification that never went out
	- EmailAdapter.h rewritten: doc comment now states the real contract (`@throws std::invalid_argument` for a null recipient, `@throws std::logic_error` for a missing service), with a `@par Preconditions` paragraph explaining why these are enforced rather than silently skipped
	- Tests updated to match: the two cases that previously expected a silent no-op (`CHECK_FALSE(run.threw)`) now assert `CHECK_THROWS_AS(..., std::logic_error)` / `CHECK_THROWS_AS(..., std::invalid_argument)`
- DONE: structure
	- NotificationService (Target interface): declares `sendNotification(const std::string& message)` as a pure virtual; virtual destructor added
	- ExternalEmailService (Adaptee): concrete class with its own `sendEmail(std::string)` method; no inheritance from NotificationService
	- EmailAdapter (Adapter): inherits NotificationService, holds a non-owning `ExternalEmailService* service` and a `Participant* recipient`. `sendNotification()` adapts the call to `service->sendEmail()`, adding the recipient's email as a prefix. Default constructor sets `service` to null; `override` keyword added
- ADD: nothing beyond what the UML shows for this pattern
- REMOVE: nothing removed relative to the UML
- DECIDE: Nothing open

---
# FACADE

- DONE: original UML operations all implemented
	- `createWorkflow(name)` and `createWorkflow()` — allocate and register a WorkflowDefinition; caller owns the returned pointer
	- `startWorkflow(definition)` — creates an instance from the definition, calls `submit()` then `activate()`, registers it, and returns it; caller owns the returned pointer
	- `assignWork(instance, item)` — validates and calls `item->execute()` after dependency checks; does not select a participant
	- `completeWork(instance, item)` — marks the item complete and resolves the instance when all items are done, with an Active→InProgress guard before resolve
	- `escalateWork(instance)` — now dispatched through `EscalateWorkCommand` via the Facade's Invoker (see COMMAND section)
	- `closeWorkflow(instance)` — now dispatched through `CloseWorkflowCommand` via the Facade's Invoker (see COMMAND section)
- DONE: Facade wired to Command/Invoker (this session)
	- The original UML drew a dependency arrow from TIVIDYFacade to Invoker, but the previous code called `instance->escalate()` and `instance->close()` directly — the Invoker was never used. That gap is now closed
	- TIVIDYFacade owns `Invoker invoker`, `EscalateWorkCommand escalateCmd`, and `CloseWorkflowCommand closeCmd` as private members
	- `escalateWork` and `closeWorkflow` set their command's receiver and call `invoker.executeCommand()`
	- `undoLastCommand()` added to the public interface and delegates to `invoker.undoCommand()`
- ADD: the Facade's public surface grew far past the 6 operations in the original UML diagram. Everything below is additional:
	- Instance-level work management: `assignWork(instance, item, participants)` (strategy-based), `completeWork(instance, item, participant)`, `addWorkItemToInstance`, `cancelWorkItem`, `reassignWorkItem`, `changeWorkItemRole`, `changeWorkItemPriority`, `addParticipantToInstance`
	- Query operations: `getHistory`, `getProgress`, `getIncompleteWorkItems`, `getAllWorkItems`, `getParticipants`, `getParticipantObjects`, `getAssignedWorkItems`, `getAssignee`
	- Registry queries: `getAllInstances`, `getAllDefinitions`, `getInstancesForDefinition`, `getRolesForDefinition`, `getParticipantsForDefinition`, `getAllParticipants`, `getDefinitionName`
	- Definition-building, all routed through a per-definition `ConcreteWorkflowBuilder` the Facade owns: `configureAssignmentStrategy`, `configureEscalationHandler`, `addRoleToDefinition`, `addWorkItemToDefinition`, `addDependencyToWorkItem`, `addPriorityToWorkItem`, `createWorkItemInDefinition`, `beginSubProcessInDefinition`, `endSubProcessInDefinition`, `createParticipantInDefinition`, `addParticipantToDefinition`, `createSequentialWorkflow` (via `WorkflowDirector`)
	- A real constructor/destructor: the Facade owns one builder per definition (`std::map<WorkflowDefinition*, ConcreteWorkflowBuilder*> builders`), created lazily by the private `builderFor()`, and deletes them in `~TIVIDYFacade()`
	- `undoLastCommand()` — not in the original UML; exposes Command/Invoker undo for escalate and close
- CHANGE: the UML's dependency arrow from TIVIDYFacade to Invoker was stale before this session; it is now accurate
- REMOVE: nothing removed relative to the UML
- DECIDE:
	- Ownership of `definitions` and `instances` is caller-managed. `~TIVIDYFacade()` only deletes its builders; nothing in the Facade deletes the `WorkflowDefinition*` or `WorkflowInstance*` it hands out. Fine for the current demo but worth noting in the report if a memory-leak check is part of marking

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

- DONE: implementation
	- WorkItemDecorator (abstract Decorator): inherits WorkflowComponent, holds a `WorkflowComponent* component` pointer. Default is non-owning; a decorator produced by `clone()` sets `ownsWrapped = true` and deletes the wrapped clone on destruction. Concrete decorators must implement `execute()` and `isComplete()`
	- PriorityDecorator (ConcreteDecorator): wraps a WorkflowComponent and adds a `priority` integer. `execute()` and `isComplete()` delegate to the wrapped component. `getPriority()` and `setPriority()` expose the value. The builder's `addPriority()` calls this and replaces the original item in the stage with the decorator
	- The priority value is written onto the underlying WorkItem as well (via `WorkItem::setPriority()`), so the priority survives cloning into a workflow instance and is visible to assignment strategies through `WorkItem::getPriority()`
- CHANGE: UML shows only `WorkItemDecorator` and one concrete decorator; the code has exactly that, so the structure matches. The ownership subtlety (ownsWrapped flag for clone-produced decorators) is not in the UML but is needed for safe tree cloning
- ADD: nothing beyond what the UML shows for this pattern
- REMOVE: nothing removed relative to the UML
- DECIDE: Nothing open