# DEVELOPER GUIDE

- How we work on TIVIDY
- Read this before your first commit

---
# SETUP

- Requirements
	- g++ (C++11)
	- CMake 3.10+
	- make
	- Doxygen
	- Graphviz
	- Valgrind

```bash
git clone https://github.com/Kristy2005/TIVIDY-Prac6.git
cd TIVIDY-Prac6
make test
```

- If `make test` is green, your setup is good

---
# DAILY COMMANDS

- `make`
	- Configure and build
- `make test`
	- Build and run all unit tests
- `make run`
	- Build and start the application
- `make docs`
	- Generate Doxygen docs
- `make clean`
	- Delete build output
- `make rebuild`
	- Clean, then build from scratch

---
# REPOSITORY LAYOUT

- `src/engine/`
	- Generic workflow engine
	- Never contains industry-specific code
- `src/industries/`
	- Industry definitions, rules, data and adapters
- `src/main.cpp`
	- Application entry point only
- `tests/`
	- doctest unit tests
- `docs/`
	- UML exports and design history
- `report/`
	- Final PDF report

- Golden rule
	- Both industries must reuse `src/engine/` without modifying it
	- Anything that mentions a specific industry belongs in `src/industries/`

---
# BRANCHING & PULL REQUESTS

- Never commit directly to `main`, it is protected
- Branch from fresh `main`, one branch per task

```bash
git checkout main && git pull
git checkout -b feature/<name>
```

- Branch prefixes
	- `feature/`
	- `fix/`
	- `test/`
	- `docs/`
	- `refactor/`
- Keep PRs small: one feature plus its tests
- Open a PR into `main` and fill in what changed and how to test it
- CI must pass (build-and-test and docs)
- At least one teammate other than the author must approve
- Merge with "Create a merge commit" so individual commits stay visible
- Delete the branch after merging

---
# COMMITS

- Small and focused
- Present tense, imperative: `Add EscalatedState transitions`
- No vague messages such as `stuff`, `fix` or `update`
- Commit under your own name and GitHub email
	- `git config user.name`
	- `git config user.email`
- Check your count with `git shortlog -sn`

---
# CODE STANDARDS

- C++11 only, no C++14/17 features
- Compiles cleanly with `-Wall -Wextra`
- Memory management
	- Normal manual memory management: `new` and `delete`
	- The owner of an object deletes it
	- Every `new` has a matching `delete`
	- Any class used as a base has a virtual destructor
	- Classes that own heap memory follow the Rule of Three
		- Copy constructor
		- Copy assignment operator
		- Destructor
- No large `if`/`switch` chains on type, use polymorphism
- Document every significant class and operation with Doxygen

```cpp
/**
 * @brief One-line summary.
 * @param x What it means.
 * @return What comes back.
 */
```

---
# TESTING

- Framework
	- doctest (`tests/doctest.h`)
	- One file per area: `tests/<area>_test.cpp`
- Every member contributes tests
- Cover
	- State transitions
	- Routing
	- Instance isolation
	- Invalid operations
- Every new feature ships with at least one test in the same PR

```cpp
#include "doctest.h"

TEST_CASE("Closed instance rejects new work") {
    // arrange, act, CHECK(...)
}
```

---
# CI

- `.github/workflows/ci.yml` runs on every push to `main` and every PR
	- Build
	- Tests
	- Valgrind
	- Doxygen
- Fix a red CI before asking for review

---
# BEFORE THE DEMO

- Fresh clone builds with `make test`
- README instructions work for both industries
- UML diagrams match the code
- Everyone can explain the whole design

---
# HOW TO WRITE TESTS WITH DOCTEST

- Every pattern you implement ships with tests in the same PR
- A passing CI run only means something if tests exist

## RULES

- One test file per pattern or area
	- `tests/observer_test.cpp`
	- `tests/command_test.cpp`
- Never define `main()` or `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` in your file
	- That lives only in `tests/test_main.cpp`
- CMake picks up new files in `tests/` automatically
	- You do not edit `CMakeLists.txt`
- Tests call the engine's public interface only
- No test may depend on industry-specific code unless it is an industry integration test

## STEP BY STEP

- Step 1: branch from fresh `main`

```bash
git checkout main && git pull
git checkout -b test/observer
```

- Step 2: create your test file

```bash
code tests/observer_test.cpp
```

- Step 3: add the include and your first test

```cpp
#include "doctest.h"
#include "engine/WorkflowInstance.h"   // include what you test

TEST_CASE("Observer is notified when state changes") {
    // 1. Arrange: build the objects
    // 2. Act: call the function under test
    // 3. Assert: CHECK the result
}
```

- Step 4: write it in three parts: Arrange, Act, Assert

```cpp
TEST_CASE("Attached observer receives a notification") {
    // Arrange
    TestObserver obs;                 // recording observer, see TEST DOUBLE
    WorkflowInstance wf(/* definition */);
    wf.attach(&obs);

    // Act
    wf.start();

    // Assert
    CHECK(obs.updateCount == 1);
}
```

- Step 5: build and run just your tests

```bash
make test
./build/tividy_tests -tc="*Observer*"
```

- Step 6: add the failure cases, for each feature write at least
	- The normal case (it works)
	- The invalid case (it is rejected and nothing changes)
	- The pointer-safety case (nothing is used after delete, nothing leaks)
- Step 7: run the memory checks before pushing

```bash
cmake -S . -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g"
cmake --build build-asan -j
./build-asan/tividy_tests
```

- Step 8: commit, push and open the PR

```bash
git add tests/observer_test.cpp
git commit -m "Add Observer tests: notify, detach and destruction safety"
git push -u origin test/observer
gh pr create --fill
```

## ASSERTIONS

- `CHECK(x == y)`
	- Normal check
	- On failure the test continues
- `REQUIRE(ptr != nullptr)`
	- Must be true or the test stops
	- Use before dereferencing
- `CHECK_FALSE(x)`
	- Expect false
- `CHECK_THROWS(expr)`
	- Expect any exception
- `CHECK_THROWS_AS(expr, ExType)`
	- Expect a specific exception type
- `CHECK_NOTHROW(expr)`
	- Expect no exception
- `CHECK(a == doctest::Approx(0.5))`
	- Compare doubles
- Use `REQUIRE` for anything that would make later lines crash if false (null pointers, empty lists)

## SUBCASE

- Each `SUBCASE` runs from a fresh copy of the setup above it

```cpp
TEST_CASE("Closed instance") {
    WorkflowInstance wf(/* definition */);
    wf.close();

    SUBCASE("rejects new work") {
        CHECK_THROWS_AS(wf.assignWork(), InvalidTransition);
    }
    SUBCASE("stays closed") {
        CHECK(wf.stateName() == "Closed");
    }
}
```

## TEST DOUBLE

- A small class that records what happened so a test can check it

```cpp
class TestObserver : public Observer {
public:
    int updateCount = 0;
    void update() override { ++updateCount; }
};
```

## USEFUL COMMANDS

```bash
./build/tividy_tests                       # run everything
./build/tividy_tests -tc="*State*"         # only tests whose name matches
./build/tividy_tests -ltc                  # list all test cases
./build/tividy_tests -s                    # also print passing assertions
./build/tividy_tests -tce="*Slow*"         # exclude matching tests
```

## COMMON MISTAKES

- Putting `main()` in a test file
	- Gives a "multiple definition of main" link error
- Testing private details
	- Test behaviour through public functions
- One giant test
	- Keep each `TEST_CASE` to one idea so a failure tells you what broke
- Vague names
	- Use `"Closed instance rejects new work"`, not `"test1"`
- Forgetting `REQUIRE` before a pointer dereference
- Tests that depend on each other or on run order
	- Each test builds its own objects
- Committing a test that fails
	- CI goes red for everyone
- Leaking in tests
	- Prefer stack objects
	- Anything you create with `new`, you `delete`