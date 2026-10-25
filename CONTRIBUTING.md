# Developer Guide

How we work on TIVIDY. Read this before your first commit.

## 1. SETUP

Requirements: g++ (C++11), CMake 3.10+, make, Doxygen, Graphviz, Valgrind.

```bash
git clone https://github.com/Kristy2005/TIVIDY-Prac6.git
cd TIVIDY-Prac6
make test
```

If `make test` is green, your setup is good.

## 2. DAILY COMMANDS

| Command | Purpose |
|---|---|
| `make` | Configure and build |
| `make test` | Build and run all unit tests |
| `make run` | Build and start the TUI |
| `make docs` | Generate Doxygen docs |
| `make clean` | Delete build output |
| `make rebuild` | Clean, then build from scratch |

## 3. REPOSITORY LAYOUT 

| Folder | Contents |
|---|---|
| `src/engine/` | Generic workflow engine. Never contains industry-specific code. |
| `src/industries/` | Industry definitions, rules, data and adapters. |
| `src/main.cpp` | The TUI entry point only. |
| `tests/` | doctest unit tests. |
| `docs/` | UML exports and design history. |
| `report/` | Final PDF report. |

Golden rule: both industries must reuse `src/engine/` without modifying it.
Anything that mentions a specific industry belongs in `src/industries/`.

## 4. BRANCHING & PULL REQUESTS

1. Never commit directly to `main`. It is protected.
2. Branch from fresh `main`, one branch per task:
   `git checkout main && git pull && git checkout -b feature/<name>`
3. Branch prefixes: `feature/`, `fix/`, `test/`, `docs/`, `refactor/`.
4. Keep PRs small: one feature plus its tests.
5. Open a PR into `main` and fill in what changed and how to test it.
6. CI must pass (build-and-test and docs).
7. At least one teammate other than the author must approve.
8. Merge with **Create a merge commit** so individual commits stay visible.
9. Delete the branch after merging.

## 5. COMMITS 

- Small and focused. Present tense, imperative: `Add EscalatedState transitions`.
- No vague messages such as `stuff`, `fix` or `update`.
- Commit under your own name and GitHub email:
  `git config user.name` and `git config user.email`.
- Check your count: `git shortlog -sn`.

## 6. CODE STANDARDS

- C++11 only. No C++14/17 features.
- Compiles cleanly with `-Wall -Wextra`.
- Any class used as a base has a virtual destructor.
- Classes that own heap memory follow the Rule of Three.
- No large `if`/`switch` chains on type. Use polymorphism.
- Document every significant class and operation with Doxygen:

```cpp
/**
 * @brief One-line summary.
 * @param x What it means.
 * @return What comes back.
 */
```

## 7. TESTING

- Framework: doctest (`tests/doctest.h`), one file per area: `tests/<area>_test.cpp`.
- Every member contributes tests.
- Cover: state transitions, routing, instance isolation, invalid operations.
- Every new feature ships with at least one test in the same PR.

```cpp
#include "doctest.h"

TEST_CASE("Closed instance rejects new work") {
    // arrange, act, CHECK(...)
}
```

## 8. CI

`.github/workflows/ci.yml` runs on every push to `main` and every PR:
build, tests, Valgrind and Doxygen. Fix a red CI before asking for review.

## 9. Before the demo

- Fresh clone builds with `make test`.
- README instructions work for both industries.
- UML diagrams match the code.
- Everyone can explain the whole design.
