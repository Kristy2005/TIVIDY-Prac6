# TIVIDY PROJECT BREAKDOWN

- Purpose
	- Explains what the COS 214 spec requires, where the code falls short and what the team still has to decide
	- Sits next to FINAL DOCUMENT CHANGES, finished items get copied into REVISIONS at the end
- How to read
	- REQUIRED: the spec says it
	- RECOMMENDED: not in the spec, but protects us from unscripted questions
	- CHANGE: text or code exists but is wrong or out of date
	- ADD: missing
	- REMOVE: must go
	- DECIDE: the team must pick an option before coding or editing
	- DONE / NOT DONE / CHECK: status of an item, CHECK means nobody has verified it yet
- Source
	- COS 214 final project spec, demo on 2 and 3 November 2026
	- The gaps below are based on the files pasted into the chat, verify them against the repo before acting on them

---
---
# 1. MENTAL MODEL

- Workflow definition
	- The blueprint: which steps exist, which role does each, dependencies, decisions, what happens on failure
	- Does not change while instances run
- Workflow instance
	- One live run of a blueprint with its own case data, participants, progress per work item, state and history
	- Example: 1 hospital definition and 3 patients is 1 definition and 3 instances that must never affect each other
- Layers
	- TUI: menus and prompts, talks only to the Facade
	- Industry packs: hospital and university, definitions, rules, data and adapters only
	- Generic engine: definitions, instances, work items, participants, routing, states, events, history
- Vocabulary
	- Stage / work item: a unit of work with its own lifecycle
	- Dependency: item B cannot start until item A is done
	- Decision / branch / routing: the next step depends on data or an outcome
	- Participant / role / assignment: who may do what and how work is given to someone
	- Exception path: rejection, failure, cancellation or escalation, with recovery
	- Event / audit history: inspectable log of what happened and when
	- Industry pack: everything specific to one industry, kept out of the engine

---
# 2. SPEC: ENGINE BEHAVIOUR

- REQUIRED: workflow definitions and independent instances
	- Changes in one instance must not unintentionally affect another
- REQUIRED: stages, work items and their lifecycle
- REQUIRED: participants, roles, responsibility, work assignment and routing
- REQUIRED: dependencies, decisions, branching and progression through work
- REQUIRED: approvals, rejection or failure, recovery or escalation
- REQUIRED: events or triggers, state changes, inspectable execution history
- REQUIRED: industry-specific rules separated from generic engine responsibilities
- OPTIONAL: parallel execution, nested work, external integrations, versioning, persistent storage
- Claimed behaviour must actually execute, a diagram alone is not enough

---
# 3. SPEC: EACH INDUSTRY

- Two genuinely different industries
	- Valid: healthcare plus legal. Not valid: two departments of one hospital
- Same reusable engine, two hard-coded programs are not allowed
- Per scenario
	- Explain industry, process, participants, input data, expected result
	- Create or select a definition and start an independent instance
	- Run several work items with dependencies and at least one routing or decision branch
	- Show a completion path and an exception path
	- Show current work, state changes and history or audit events
	- Name the generic engine classes both industries reuse without modification

---
# 4. SPEC: PATTERNS

- At least 10 distinct, meaningful GoF patterns
- At least 2 creational, 2 structural, 2 behavioural
- At least 5 from: Observer, Iterator, Mediator, Command, Adapter, Chain of Responsibility, Builder, Interpreter, Bridge, Facade, Visitor, Proxy, Singleton, Flyweight
- Patterns must solve real workflow problems and be executed inside the running system
- Avoid large centralised type-checking branches where polymorphism fits
- Pattern justifications, participant mappings and pattern diagrams must match the real classes

---
# 5. SPEC: UML AND DOCUMENTATION

- Three substantial activity diagrams: generic execution, another generic engine process, one selected scenario
- One complete system class diagram
- At least two sequence diagrams and at least one state diagram
- Add or change diagrams to explain the second industry and the real runtime behaviour
- Colour legend, clear ownership and relationships, documented design decisions and revisions
- Functional requirements updated to match what was built, linked to design and implementation
- All diagrams agree with each other and with the C++ code
- Visual Paradigm project files and readable exports

---
# 6. SPEC: IMPLEMENTATION AND TUI

- C++11 engine, genuinely interactive TUI (a GUI is optional and earns no bonus)
- A marker must be able to
	- start an instance
	- inspect available work
	- take actions
	- observe routing and state changes
	- view history
	- use both industries through the running interface
- Printed menus without real interaction, recordings and slides do not count
- Realistic test data and reasonable error handling for invalid transitions
- External services may be documented test adapters or stubs
- Repeatable build and run instructions

---
# 7. SPEC: DEVELOPMENT PRACTICES

- Git
	- At least 10 meaningful commits per member
	- At least 3 closed pull requests across the group
	- Documented branching and review process
- Doxygen on significant classes and non-obvious logic
- README with dependencies, build, run and test instructions for both industries
- Real framework tests (doctest)
	- transitions, routing, instance isolation, invalid operations
	- every member contributes tests
- Bonus (cannot replace core work): CI/CD, Docker, automated quality checks, extra integration tests

---
# 8. SPEC: REPORT AND REPOSITORY

- PDF report
	- Team information, contribution statement, GitHub link
	- At least 4 credible references, assumptions, TIVIDY boundaries
	- Both industries: processes, participants, branching, failure paths, outcomes
	- Updated requirements, UML, pattern roles and justifications
	- Implementation explanation and design decision history
	- Traceability: requirement to class to pattern to test
	- Build, run and test instructions, Git and engineering evidence
- Repository
	- Source, build files, tests, necessary data
	- README.md, docs/, UML exports
	- Final PDF under report/
	- Optional engineering configuration

---
# 9. SPEC: DEMO (30 MINUTES)

- 0-4 min: reusable engine and both industries
- 4-12 min: industry 1 live, including a decision and an exception
- 12-20 min: industry 2 live on the same engine
- 20-25 min: GoF collaborations, diagrams, automated tests
- 25-30 min: questions, code inspection, individual understanding
- Markers may ask for unscripted interactions and inspect code
- Every member attends and must understand the whole design

---
---
# RUNTIME FLEXIBILITY

- Spec wording
	- "Create or select a workflow definition and start an independent workflow instance"
	- A marker must be able to start an instance, inspect available work and take actions
	- "Markers may request unscripted interactions"
	- Editing definitions live is not stated anywhere
- Level 1 (REQUIRED)
	- Pick a preloaded definition, start instances, act on work items one by one, see state and history
- Level 2 (RECOMMENDED)
	- Start extra instances, add a participant, add or reassign a work item on a running instance
	- Reject, escalate, cancel
- Level 3 (OPTIONAL)
	- Define a brand-new workflow from scratch through a TUI wizard or a data file
	- Strongest proof of one generic engine, a third industry becomes a new file
- DECIDE: which level the team aims for
	- Suggested: levels 1 and 2, level 3 only if time remains

---
# GAPS IN THE CURRENT CODE (30 OCTOBER 2026)

- GAP 1: the scenarios are scripts
	- Evidence: runHospitalAdmissionScenario and runUniversityComplaintScenario build everything and run start to finish
	- Spec link: a marker must start an instance and act step by step
	- CHANGE: split definition setup from TUI-driven actions (start instance, list work, complete, reject, escalate, close)
	- DECIDE: where definitions live (C++ factory functions, Builder/Director, or data files)
- GAP 2: definition and instance are not separated
	- Evidence: WorkflowInstance holds only currentState, definition and observers, work items live in the definition's tree
	- Risk: two instances of one definition would share work item progress, which breaks isolation
	- CHANGE: the instance owns its runtime work items, case data, assignees and history
	- ADD: isolation tests with two instances of the same definition
	- DECIDE: how runtime items are created from templates (copy, Factory Method, Builder)
- GAP 3: no dependencies or decision branches, hospital has no exception path
	- Evidence: WorkflowStage runs items in order, the hospital scenario only has a completion path, the university scenario calls escalate() but stays linear afterwards
	- ADD: dependencies between work items
	- ADD: at least one data-driven branch per industry
	- ADD: an exception path in both industries
	- DECIDE: how decisions are expressed (predicate classes, Strategy, Interpreter)
- GAP 4: no real history
	- Evidence: ProgressObserver prints "Workflow state changed." without naming the state, OBSERVER.md already lists HistoryObserver as an audit trail that is not done
	- ADD: audit event log per instance (time, event, work item, participant, from and to state)
	- ADD: history view in the TUI
- GAP 5: most patterns are not reachable from the running app
	- Evidence: the scenarios use State, Observer, Composite, Strategy and Chain of Responsibility
	- Evidence: Command/Invoker, Memento/WorkflowHistory, Decorator, Builder/Director, Facade and the Adapter are not used by the scenarios or main.cpp
	- Spec link: patterns must be executed within the coherent system
	- CHANGE: route TUI actions through Facade, then Commands (undo through Memento), then the instance
	- CHANGE: build definitions with Builder/Director, send notifications through the Adapter, use Decorator on real work items
- GAP 6: the Facade is bypassed and has a bug
	- Evidence: the scenarios never call TIVIDYFacade
	- Evidence: completeWork() calls item->complete() and then instance->resolve(), so finishing one item resolves the whole instance
	- Evidence: assignWork() only calls item->execute()
	- CHANGE: completeWork only completes the item, the engine resolves the instance when all required work is done
	- ADD: tests for both
- GAP 7: industry wording inside generic classes
	- Evidence: state headers and WorkflowInstance docs say "the student retracts the complaint"
	- CHANGE: neutral wording (case, requester) in engine comments and Doxygen
- GAP 8: creational pattern count
	- Known patterns
		- Creational: Builder
		- Structural: Composite, Adapter, Facade, Decorator
		- Behavioural: State, Strategy, Chain of Responsibility, Command, Observer, Memento
	- Total is 11, and 6 come from the required list (Observer, Command, Adapter, Chain of Responsibility, Builder, Facade)
	- Problem: only one creational pattern, the spec needs at least 2
	- DECIDE: a second creational pattern with a real job (Prototype or Factory Method for runtime work items, Singleton for a registry or audit log)

---
# TARGET SHAPE (SKETCH, NOT A DECISION)

- Definition side
	- Templates for work items with required role, dependencies, decision rules and failure handling
	- Built by Builder/Director from code or from a data file
- Instance side
	- Owns runtime work items created from the templates, case data, assignees, history log and State
- Engine services
	- Assignment: Strategy
	- Decision rules: small predicate classes, Interpreter only if rules must be editable at runtime
	- Escalation: Chain of Responsibility
	- TUI actions: Facade, Commands, undo through Memento
	- Audit log, progress and notifications: Observers, notifications through the Adapter
- Industry packs
	- Definitions, rules, adapters and sample data only
- Design checks
	- Third-industry test: can a third industry be added with new files only and no edits to engine classes
	- Delete test: with both industry files deleted, does the engine still build and pass its tests
	- The list of generic engine classes for the demo comes out of these two checks

---
# TUI: WHAT A MARKER MIGHT DO

- NOT DONE
	- Start an instance of either industry
	- Start a second instance of the same definition and show the first is unaffected
	- List instances and select one
	- List available work (dependencies satisfied, assigned by role)
	- Complete a work item
	- Reject or fail a work item
	- Escalate, cancel or close an instance
	- Try an invalid action (complete an item whose dependency is not done, resolve a closed instance) and see a clear error with no state change
	- View the history or audit trail of one instance
	- Level 2 and up: add a participant or a work item to a running instance

---
# DIAGRAMS AND TRACEABILITY

- Activity diagrams (suggested content)
	- Generic execution: start instance, assign, complete, decide, resolve or exception
	- Another generic engine process: escalation or definition building
	- One scenario: hospital admission or university complaint
- Class diagram: complete, with ownership and multiplicity, definition side and instance side clearly separate
- Sequence diagrams (suggested): start instance, complete a work item with a decision, escalate
- State diagram: instance lifecycle (and work item lifecycle if it is separate)
- Keep the colour legend and the design decision log
- Traceability table (fill in as the design settles)

| Requirement | Class(es) | Pattern | Test |
|---|---|---|---|
| Instances are isolated | WorkflowInstance, per-instance work items | (decide) | isolation test |
| Escalation when work is stuck | Handler chain | Chain of Responsibility | escalation test |
| | | | |

---
# ADMIN CHECKLIST

- CHECK: every member has at least 10 meaningful commits
- CHECK: at least 3 closed pull requests across the group
- CHECK: every member has written tests
- CHECK: Doxygen builds and the README covers both industries
- CHECK: report has at least 4 credible references
- CHECK: Visual Paradigm files and UML exports are in the repo
- CHECK: CI with Valgrind is demo-able (counts as bonus work)
- CHECK: deadline for the report and repo, the spec only gives the demo dates

---
# TIMELINE

- Today is Friday 30 October 2026, demos are Monday 2 and Tuesday 3 November
- Suggested order
	- 1. Fix the definition and instance seam, route actions through the Facade
	- 2. Make the TUI drive instances step by step, with history
	- 3. Add a decision branch and an exception path to both industries
	- 4. Add the level 2 runtime actions
	- 5. Sync UML, report, tests and README
- Keep make test and make valgrind green throughout

---
# QUESTIONS FOR TUTOR OR AL

- DECIDE: is a data-file definition acceptable as the generic mechanism
- DECIDE: does the exception path need dedicated work items or does an escalation count
- DECIDE: is runtime definition editing expected, or is select or create enough

---
---
# REVISIONS

- Format of an entry
	- DATE / AREA
	- CHANGE:
	- ADD:
	- REMOVE:
	- DECIDE:
	- WHY:
	- UML updated: yes / no
	- Tests updated: yes / no

- Done so far and not recorded in FINAL DOCUMENT CHANGES (add dates, correct anything that is wrong)
	- TUI
		- ADD: ncurses two-panel menu in src/main.cpp replacing the numbered std::cin menu
		- ADD: accent colour per scenario, animated lifecycle strip, status bar with clock
		- ADD: scenarios still print as plain text after the TUI steps aside
		- WHY: readability and a more interactive demo
	- TUI documentation viewer
		- ADD: View documentation builds Doxygen when docs are missing, prints the file:// link and opens the browser
		- ADD: explorer.exe is used inside WSL, xdg-open elsewhere
		- CHECK: Doxyfile OUTPUT_DIRECTORY set to docs
	- Build
		- CHANGE: CMakeLists links Curses to the tividy target with PRIVATE
		- ADD: Makefile valgrind target
		- CHANGE: Makefile docs prints the link, clean removes docs/html and docs/latex
	- Tests
		- ADD: observer_test.cpp covering Observer and Adapter
		- CHANGE: EmailAdapter tests expect exceptions for a null recipient and a missing service
		- CHANGE: makeInstanceIn in StateTests uses a static state, which removed 36 test-only leaks
	- Console.h
		- ADD: ANSI colour constants used by the scenario output

- Entries from here on
	-