# TIVIDY

TIVIDY is a generic case-processing workflow management system developed for COS 214 Practical 6.

The project provides one reusable workflow engine that can be configured for different industries without rewriting the underlying workflow-management logic.

The current implementation demonstrates the engine using two different industries:

1. University Student Complaint Management
2. Hospital Patient Admission Management

The same workflow engine, workflow components and design-pattern infrastructure are reused for both scenarios.

---

## Overview

TIVIDY manages workflows involving:

- Case submission
- Work-item assignment
- Processing
- Dependencies between work items
- Routing and decisions
- Escalation
- Priorities
- Notifications
- Progress monitoring
- Audit history
- Resolution
- Closure

A central design principle of TIVIDY is the distinction between a `WorkflowDefinition` and a `WorkflowInstance`.

A `WorkflowDefinition` describes how a workflow is structured and configured.

A `WorkflowInstance` represents one running case created from that definition.

Multiple independent workflow instances can therefore be created from the same reusable definition without their state, progress or history being mixed.

---

## Industry Demonstrations

### University Student Complaint Management

The university workflow demonstrates the handling of a student complaint.

Typical work includes:

- Recording the complaint
- Validating the complaint and supporting evidence
- Investigating the complaint
- Routing the case for additional review where required
- Escalating the case through a Chain of Responsibility
- Reviewing the outcome
- Notifying the student
- Closing the complaint

The workflow includes dependencies, a routing decision, normal processing and an escalation path.

### Hospital Patient Admission Management

The hospital workflow demonstrates the admission of a patient.

Typical work includes:

- Registering the patient
- Verifying documents
- Verifying payment or admission information
- Determining whether emergency clinical review is required
- Approving the admission
- Allocating a bed

Urgent and normal admissions follow different routes while using the same generic TIVIDY workflow engine.

---

## Requirements

TIVIDY is written in C++11.

The following tools and packages are required for a normal Ubuntu build:

- `g++` with C++11 support
- CMake 3.10 or newer
- ncurses development package
- Git

Optional development and documentation tools:

- Doxygen
- Graphviz
- Valgrind

The project includes doctest, so a separate doctest installation is not required.

On Ubuntu, the required packages can be installed with:

```bash
sudo apt update

sudo apt install build-essential cmake libncurses-dev git
```

Optional tools can be installed with:

```bash
sudo apt install doxygen graphviz valgrind
```

---

## Building the Project

Clone the repository and move into the project directory:

```bash
git clone https://github.com/Kristy2005/TIVIDY-Prac6.git
cd TIVIDY-Prac6
```

Build the project with:

```bash
make build
```

The build output is created in the `build/` directory.

---

## Running TIVIDY

Run the application with:

```bash
make run
```

TIVIDY provides an interactive terminal interface from which the available workflow demonstrations can be selected.

### TUI Menu and Controls

The main menu has four entries:

| Key | Menu entry | What it does |
|---|---|---|
| `1` | Hospital admission | Runs the Hospital Patient Admission workflow |
| `2` | University complaint | Runs the University Student Complaint workflow |
| `3` | View documentation | Opens the generated Doxygen documentation (runs Doxygen first if needed) |
| `4` | Exit | Closes the demo |

| Key | Action |
|---|---|
| `Up` / `Down` (or `k` / `j`) | Move the selection |
| `Enter` | Run the selected entry |
| `1` to `4` | Jump to that entry and run it immediately |
| `q` / `Q` | Quit |

Each scenario asks for a decision while it runs (for example, whether the hospital patient is an urgent admission, or whether the university complaint needs escalation). Answer `y` or `n` and press `Enter`. When a scenario ends, press `Enter` to return to the menu.

The terminal must be at least 72 columns by 20 rows. If it is smaller, the interface asks you to resize it.

---

## Running the Tests

Run the automated test suite with:

```bash
make test
```

The project uses the doctest testing framework together with CTest.

Dedicated tests cover the workflow engine and the design patterns used throughout TIVIDY.

To run CTest directly:

```bash
ctest --test-dir build --output-on-failure
```

---

## Memory Checking with Valgrind

Run the test suite under Valgrind with:

```bash
make valgrind
```

Valgrind is used to detect memory leaks and invalid memory access.

A successful result should end with:

```text
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors
```

---

## Generating Documentation

TIVIDY uses Doxygen for source-code documentation.

Generate the documentation with:

```bash
make docs
```

Doxygen and Graphviz must be installed for documentation generation.

The generated HTML documentation is placed in the `html/` folder in the project root. Open `html/index.html` in a browser, or choose **View documentation** from the TUI menu. `make clean` removes the `html/` folder.

---

## Repository Structure

The repository is organised approximately as follows:

```text
TIVIDY-Prac6/
|
|-- .github/
|   |-- workflows/
|       GitHub Actions continuous integration workflows
|
|-- docs/
|   UML diagrams, research, design documentation and
|   supporting project files
|
|-- report/
|   Final project report material
|
|-- src/
|   |-- engine/
|   |   Generic workflow engine and design-pattern classes
|   |
|   |-- industries/
|   |   University and hospital workflow demonstrations
|   |
|   |-- main.cpp
|       Application entry point
|
|-- tests/
|   Automated doctest test files
|
|-- CMakeLists.txt
|   CMake build configuration
|
|-- Makefile
|   Build, run, test, documentation and Valgrind commands
|
|-- Dockerfile
|   Docker build configuration
|
|-- .dockerignore
|   Docker exclusions
|
|-- Doxyfile
|   Doxygen documentation configuration
|
|-- README.md
|   Main project documentation
|
|-- README_SCENARIOS.txt
|   Additional workflow scenario information
|
|-- CONTRIBUTING.md
|   Contribution guidance
|
|-- CHANGES.md
|   Recorded project changes
|
|-- PROJECT_BREAKDOWN.md
|   Project breakdown and supporting planning

```
## Design Patterns

TIVIDY uses GoF design patterns to keep workflow structure, behaviour and integrations separate.

| Pattern | Main Classes | Purpose in TIVIDY |
|---|---|---|
| Builder | `WorkflowBuilder`, `ConcreteWorkflowBuilder`, `WorkflowDirector` | Constructs and configures workflow definitions |
| Prototype | `WorkflowComponent`, `WorkItem`, `WorkflowStage`, `SubProcess` | Creates independent workflow structures for running instances |
| Composite | `WorkflowComponent`, `WorkflowStage`, `SubProcess`, `WorkItem` | Represents nested workflow structures uniformly |
| Decorator | `WorkItemDecorator`, `PriorityDecorator` | Adds behaviour or metadata such as priority without changing `WorkItem` |
| Adapter | `EmailAdapter`, `ExternalEmailService` | Adapts an external email service to TIVIDY's notification interface |
| State | `WorkflowState` and concrete state classes | Controls behaviour according to the current workflow state |
| Strategy | `AssignmentStrategy`, `RoleBasedAssignment`, `WorkloadBasedAssignment` | Allows different work-assignment algorithms |
| Observer | `Observer`, `ProgressObserver`, `HistoryObserver`, `NotificationObserver` | Reacts to workflow changes without tightly coupling observers to the workflow |
| Command | `Command` and concrete workflow commands | Encapsulates workflow actions such as assignment, completion and escalation |
| Memento | `WorkflowMemento`, `WorkflowHistory` | Preserves workflow state for restoration where required |
| Chain of Responsibility | `Handler`, `ApprovalHandler`, `EscalationHandler`, `FinalDecisionHandler` | Passes escalation requests through a configurable sequence of handlers |
| Facade | `TIVIDYFacade` | Provides a simplified interface to the workflow engine |

The patterns are designed to work together rather than operate as isolated examples.

For example, a workflow may be built using Builder, cloned for an independent instance using Prototype, assign work using Strategy, change behaviour through State, execute operations through Command and escalate requests through Chain of Responsibility.

---

## Adding a New Industry or Workflow Definition

TIVIDY is designed so that a new industry can reuse the existing engine.

A new workflow normally requires the following:

1. Define the required `WorkItem` objects.
2. Add the work items to the workflow structure.
3. Define dependencies between work items where required.
4. Configure an assignment strategy.
5. Configure escalation handlers if the workflow supports escalation.
6. Configure priorities or other decorators where required.
7. Create the `WorkflowDefinition`.
8. Create one or more independent `WorkflowInstance` objects from the definition.
9. Run the workflow through the TIVIDY engine or Facade.

Industry-specific logic should remain outside the generic engine wherever possible.

The existing university and hospital workflows provide examples of two different industries using the same underlying engine.

---

## Running TIVIDY with Docker

TIVIDY can be built and executed inside a Docker container using Ubuntu 24.04 LTS.

Docker provides the required build tools and dependencies, including CMake, ncurses, Doxygen, Graphviz and Valgrind.

### Prerequisites

- Docker Desktop installed and running
- Git installed to clone the repository

### Build the Docker Image

From the root directory of the repository, run:

```bash
docker build -t tividy .
```

This builds the TIVIDY application and creates a Docker image named `tividy`.

### Run TIVIDY

Run:

```bash
docker run --rm -it tividy
```

The `-it` flags enable interactive terminal input and output required by the terminal interface.

### Run the Automated Tests

Run:

```bash
docker run --rm tividy ctest --test-dir build --output-on-failure
```

CTest executes the automated test suite and reports any failures.

### Docker Configuration

The repository includes:

- `Dockerfile` - Defines the Ubuntu environment, installs dependencies and builds TIVIDY.
- `.dockerignore` - Excludes unnecessary files and generated build artefacts from the Docker context.

The application starts by default when the container is launched without another command.

---

## Continuous Integration

The repository uses GitHub Actions to automatically build and test the project when changes are submitted.

The CI workflow should remain green before code is merged into the main branch.

[![CI](https://github.com/Kristy2005/TIVIDY-Prac6/actions/workflows/ci.yml/badge.svg)](https://github.com/Kristy2005/TIVIDY-Prac6/actions/workflows/ci.yml)

The workflow (`.github/workflows/ci.yml`) builds the project, runs the tests, runs the tests under Valgrind and builds the Doxygen documentation on every push to `main` and on every pull request.

---

## Troubleshooting

### Terminal Size

The interactive interface should be run in a terminal of at least:

```text
72 columns x 20 rows
```

If the interface appears cut off or incorrectly aligned, enlarge the terminal window and run the application again.

### ncurses Is Missing

On Ubuntu, install the ncurses development package:

```bash
sudo apt install libncurses-dev
```

Then rebuild the project.

### Running Under WSL

When using Windows Subsystem for Linux, build and execute TIVIDY from the Linux terminal.

If external documentation or browser-opening functionality is used, browser behaviour may differ between native Linux and WSL.

### Building Without the TUI

The engine and automated tests should remain separable from the ncurses interface so that engine tests can run in non-interactive environments such as CI.

### Build Problems

A clean rebuild can be performed with:

```bash
rm -rf build
make build
```

---

## Team Contributions

### Task 1 - Research

**Contributors:** LM Kabongo, Joshua Peter

LM Kabongo and Joshua Peter worked on research into workflow-management systems and the concepts relevant to TIVIDY. This included workflow definitions and instances, activities, work items, assignment, routing, participants, dependencies, events, auditing, monitoring, reusability and workflow versioning.

### Task 2 - Initial Scope and Scenario

**Contributor:** Kristy Heesen

Kristy Heesen developed the initial scope of TIVIDY and defined the University Student Complaint Management scenario. This included distinguishing workflow definitions from running workflow instances and establishing the responsibilities and boundaries of the generic workflow-management system.

### Task 3 - UML Activity Diagrams

**Contributors:** Lindokuhle Skosana, Joshua Peter

Lindokuhle Skosana and Joshua Peter worked on UML Activity Diagrams for the generic TIVIDY workflows and the University Student Complaint Management scenario.

### Task 4 - Design Patterns

**Contributors:** Kristy Heesen, Musa Msibi, Lindokuhle Skosana

Kristy Heesen, Musa Msibi and Lindokuhle Skosana worked on identifying and justifying the GoF design patterns used in TIVIDY. This included defining the design problems, pattern participants, collaborations, variation points and how the patterns work together within the workflow-management system.

### Task 5 - UML Class Diagram

**Contributor:** Kristy Heesen

Kristy Heesen worked on developing the complete UML Class Diagram for TIVIDY. This included the main classes, relationships, multiplicities, attributes, operations, ownership relationships and collaborations between the different design patterns and subsystems.

### Task 6 - Runtime Behaviour Diagrams

**Contributors:** LM Kabongo, Musa Msibi

LM Kabongo and Musa Msibi worked on the UML runtime-behaviour diagrams. LM Kabongo developed the sequence diagram showing completion of a `WorkItem` and determining what happens next, as well as the `WorkflowInstance` State Diagram. Musa Msibi developed the sequence diagram showing the escalation process.

### Task 7 - Design Decisions and Revisions

**Contributors:** Entire Team

The team documented the major design decisions and revisions made throughout the practical, including changes to State, Chain of Responsibility, Command and other patterns as the design was refined.

### Task 8 - GitHub, Testing and Documentation

**Contributors:** Entire Team

The team contributed to the GitHub repository, automated testing and project documentation, including research, UML diagrams, design decisions, the README and supporting project material.

---

## Final Verification

Before submission, the project should be tested from a fresh clone on Ubuntu using only the instructions in this README.

The following should all work without requiring undocumented steps:

```bash
make build
make test
make run
make valgrind
make docs
```

Both the University Student Complaint workflow and Hospital Patient Admission workflow should run successfully.

The commands above were copy-pasted and run from a fresh clone on Ubuntu 24.04 (commit `28d9a27`) on 1 November 2026: `make build`, `make test`, `ctest --test-dir build --output-on-failure`, `make valgrind` and `make docs` all succeeded, and `make run` started the TUI and ran a scenario. Re-run them from a fresh clone after the last merge before submission.

---

## Project

COS 214 - Software Design

Practical 6 - TIVIDY Generic Case-Processing Workflow Management System
