/**
 * @file StateTests.cpp
 * @brief Automated unit tests for the State pattern (WorkflowState hierarchy).
 *
 * Covers the five concrete states: SubmittedState, ActiveState,
 * InProgressState, ResolvedState and ClosedState.
 *
 * What is tested, in the order of the COS 214 testing requirements:
 *  - valid transitions (each state moves the instance to the right next state)
 *  - invalid operations (each state rejects what it must reject with
 *    std::logic_error and leaves the instance unchanged)
 *  - observers (a valid transition notifies once, a rejected one never does)
 *  - null guards (std::invalid_argument)
 *  - full lifecycles (completion path and retraction path)
 *  - instance isolation (changing one instance never affects another)
 *
 * Framework: doctest (single header, doctest.h).
 *
 * @note Two small things are needed from WorkflowInstance that are not in the
 *       current UML: a default constructor and a getState() accessor. If your
 *       names differ, change only makeInstanceIn() and isIn() below.
 */

#include "doctest.h"

#include <memory>
#include <string>
#include <stdexcept>

#include "engine/WorkflowInstance.h"
#include "engine/WorkflowObserver.h"
#include "engine/SubmittedState.h"
#include "engine/ActiveState.h"
#include "engine/InProgressState.h"
#include "engine/ResolvedState.h"
#include "engine/ClosedState.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowStage.h"
#include "engine/WorkItem.h"

using namespace tividy;

namespace {

// ---------------------------------------------------------------- helpers

/** A WorkflowInstance operation that takes no arguments, e.g. activate(). */
typedef void (WorkflowInstance::*Operation)();

/** An operation together with its name, so failures say which one broke. */
struct NamedOperation {
	Operation op;
	const char* name;
};

const NamedOperation ACTIVATE        = { &WorkflowInstance::activate,        "activate" };
const NamedOperation START_PROCESSING = { &WorkflowInstance::startProcessing, "startProcessing" };
const NamedOperation ESCALATE        = { &WorkflowInstance::escalate,        "escalate" };
const NamedOperation RESOLVE         = { &WorkflowInstance::resolve,         "resolve" };
const NamedOperation CLOSE           = { &WorkflowInstance::close,           "close" };

/** Observer that only counts how often it was told about a change. */
class CountingObserver : public WorkflowObserver {
public:
	CountingObserver() : calls(0), lastInstance(nullptr) {}

	virtual void update(WorkflowInstance* instance) override {
		++calls;
		lastInstance = instance;
	}

	int calls;
	WorkflowInstance* lastInstance;
};

/** Builds an instance that is placed directly into state S. */
template <typename S>
std::unique_ptr<WorkflowInstance> makeInstanceIn() {
	std::unique_ptr<WorkflowInstance> instance(new WorkflowInstance());
	static S s;  // one shared object of type S for all instances in this state
	instance->setState(&s);  // setState() notifies observers, but there are none yet
	return instance;
}

/** True when the instance is currently in state S. */
template <typename S>
bool isIn(WorkflowInstance& instance) {
	return dynamic_cast<S*>(instance.getState()) != nullptr;
}

/**
 * Checks that state S rejects the operation: it throws std::logic_error,
 * the instance stays in S and no observer is notified.
 */
template <typename S>
void checkRejected(const NamedOperation& operation) {
	INFO("operation: " << std::string(operation.name));
	CountingObserver observer;
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<S>();
	instance->attach(&observer);

	CHECK_THROWS_AS(((*instance).*(operation.op))(), std::logic_error);
	CHECK(isIn<S>(*instance));
	CHECK(observer.calls == 0);
}

/**
 * Checks that the operation moves the instance from state From to state To
 * and tells the observer exactly once.
 */
template <typename From, typename To>
void checkTransition(const NamedOperation& operation) {
	INFO("operation: " << std::string(operation.name));
	CountingObserver observer;
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<From>();
	instance->attach(&observer);

	CHECK_NOTHROW(((*instance).*(operation.op))());
	CHECK(isIn<To>(*instance));
	CHECK(observer.calls == 1);
	CHECK(observer.lastInstance == instance.get());
}

} // namespace

// ============================================================ valid transitions

TEST_CASE("SubmittedState: activate moves the instance to Active") {
	checkTransition<SubmittedState, ActiveState>(ACTIVATE);
}

TEST_CASE("ActiveState: startProcessing moves the instance to In Progress") {
	checkTransition<ActiveState, InProgressState>(START_PROCESSING);
}

TEST_CASE("ActiveState: close moves the instance to Closed (student retracts)") {
	checkTransition<ActiveState, ClosedState>(CLOSE);
}

TEST_CASE("InProgressState: resolve moves the instance to Resolved") {
	checkTransition<InProgressState, ResolvedState>(RESOLVE);
}

TEST_CASE("InProgressState: close moves the instance to Closed (retracted or rejected)") {
	checkTransition<InProgressState, ClosedState>(CLOSE);
}

// ========================================================== invalid operations

TEST_CASE("SubmittedState rejects everything except activate") {
	SUBCASE("startProcessing") { checkRejected<SubmittedState>(START_PROCESSING); }
	SUBCASE("escalate")        { checkRejected<SubmittedState>(ESCALATE); }
	SUBCASE("resolve")         { checkRejected<SubmittedState>(RESOLVE); }
	SUBCASE("close")           { checkRejected<SubmittedState>(CLOSE); }
}

TEST_CASE("ActiveState rejects activate, escalate and resolve") {
	SUBCASE("activate") { checkRejected<ActiveState>(ACTIVATE); }
	SUBCASE("escalate") { checkRejected<ActiveState>(ESCALATE); }
	SUBCASE("resolve")  { checkRejected<ActiveState>(RESOLVE); }
}

TEST_CASE("InProgressState rejects activate and startProcessing") {
	SUBCASE("activate")        { checkRejected<InProgressState>(ACTIVATE); }
	SUBCASE("startProcessing") { checkRejected<InProgressState>(START_PROCESSING); }
}

TEST_CASE("ResolvedState is final: it rejects every operation") {
	SUBCASE("activate")        { checkRejected<ResolvedState>(ACTIVATE); }
	SUBCASE("startProcessing") { checkRejected<ResolvedState>(START_PROCESSING); }
	SUBCASE("escalate")        { checkRejected<ResolvedState>(ESCALATE); }
	SUBCASE("resolve")         { checkRejected<ResolvedState>(RESOLVE); }
	SUBCASE("close")           { checkRejected<ResolvedState>(CLOSE); }
}

TEST_CASE("ClosedState is final: it rejects every operation") {
	SUBCASE("activate")        { checkRejected<ClosedState>(ACTIVATE); }
	SUBCASE("startProcessing") { checkRejected<ClosedState>(START_PROCESSING); }
	SUBCASE("escalate")        { checkRejected<ClosedState>(ESCALATE); }
	SUBCASE("resolve")         { checkRejected<ClosedState>(RESOLVE); }
	SUBCASE("close")           { checkRejected<ClosedState>(CLOSE); }
}

// ================================================================ null guards

TEST_CASE("States that change the instance reject a null instance") {
	SUBCASE("SubmittedState::activate") {
		SubmittedState state;
		CHECK_THROWS_AS(state.activate(nullptr), std::invalid_argument);
	}
	SUBCASE("ActiveState::startProcessing") {
		ActiveState state;
		CHECK_THROWS_AS(state.startProcessing(nullptr), std::invalid_argument);
	}
	SUBCASE("ActiveState::close") {
		ActiveState state;
		CHECK_THROWS_AS(state.close(nullptr), std::invalid_argument);
	}
	SUBCASE("InProgressState::resolve") {
		InProgressState state;
		CHECK_THROWS_AS(state.resolve(nullptr), std::invalid_argument);
	}
}

// ================================================================= lifecycles

TEST_CASE("Lifecycle: completion path Submitted -> Active -> In Progress -> Resolved") {
	CountingObserver observer;
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<SubmittedState>();
	instance->attach(&observer);

	instance->activate();
	CHECK(isIn<ActiveState>(*instance));

	instance->startProcessing();
	CHECK(isIn<InProgressState>(*instance));

	instance->resolve();
	CHECK(isIn<ResolvedState>(*instance));

	CHECK(observer.calls == 3);   // one notification per transition
}

TEST_CASE("Lifecycle: retraction path Submitted -> Active -> Closed") {
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<SubmittedState>();

	instance->activate();
	instance->close();

	CHECK(isIn<ClosedState>(*instance));
}

TEST_CASE("Lifecycle: a resolved instance stays resolved whatever is attempted") {
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<SubmittedState>();
	instance->activate();
	instance->startProcessing();
	instance->resolve();

	CHECK_THROWS_AS(instance->activate(), std::logic_error);
	CHECK_THROWS_AS(instance->startProcessing(), std::logic_error);
	CHECK_THROWS_AS(instance->resolve(), std::logic_error);
	CHECK_THROWS_AS(instance->close(), std::logic_error);
	CHECK(isIn<ResolvedState>(*instance));
}

TEST_CASE("Lifecycle: a closed instance cannot be reopened") {
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<SubmittedState>();
	instance->activate();
	instance->close();

	CHECK_THROWS_AS(instance->activate(), std::logic_error);
	CHECK_THROWS_AS(instance->startProcessing(), std::logic_error);
	CHECK(isIn<ClosedState>(*instance));
}

TEST_CASE("Lifecycle: a skipped step is rejected and nothing changes") {
	// A submitted case cannot jump straight to In Progress or Resolved.
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<SubmittedState>();

	CHECK_THROWS_AS(instance->startProcessing(), std::logic_error);
	CHECK_THROWS_AS(instance->resolve(), std::logic_error);
	CHECK(isIn<SubmittedState>(*instance));
}

// ============================================================ instance isolation

TEST_CASE("Instance isolation: moving one instance does not affect another") {
	CountingObserver observerA;
	CountingObserver observerB;
	std::unique_ptr<WorkflowInstance> a = makeInstanceIn<SubmittedState>();
	std::unique_ptr<WorkflowInstance> b = makeInstanceIn<SubmittedState>();
	a->attach(&observerA);
	b->attach(&observerB);

	a->activate();
	a->startProcessing();

	CHECK(isIn<InProgressState>(*a));
	CHECK(isIn<SubmittedState>(*b));      // b has not moved
	CHECK(observerA.calls == 2);
	CHECK(observerB.calls == 0);          // b's observer heard nothing
}

TEST_CASE("Instance isolation: closing one instance leaves another running") {
	std::unique_ptr<WorkflowInstance> a = makeInstanceIn<ActiveState>();
	std::unique_ptr<WorkflowInstance> b = makeInstanceIn<ActiveState>();

	a->close();

	CHECK(isIn<ClosedState>(*a));
	CHECK(isIn<ActiveState>(*b));
	CHECK_NOTHROW(b->startProcessing());  // b can still progress normally
	CHECK(isIn<InProgressState>(*b));
}

TEST_CASE("Instance isolation: a rejected action on one instance does not affect another") {
	std::unique_ptr<WorkflowInstance> a = makeInstanceIn<ResolvedState>();
	std::unique_ptr<WorkflowInstance> b = makeInstanceIn<InProgressState>();

	CHECK_THROWS_AS(a->close(), std::logic_error);

	CHECK(isIn<ResolvedState>(*a));
	CHECK(isIn<InProgressState>(*b));
}

// ================================================================== pending work

// escalate() is part of the design (InProgressState::escalate runs the escalation
// handler chain and the instance STAYS In Progress, there is no EscalatedState).
// InProgressState does not override escalate() yet, so this test is skipped.
// Remove "* doctest::skip()" once InProgressState::escalate() exists and the
// instance has an escalation handler configured.
TEST_CASE("InProgressState: escalate keeps the instance In Progress" * doctest::skip()) {
	std::unique_ptr<WorkflowInstance> instance = makeInstanceIn<InProgressState>();

	CHECK_NOTHROW(instance->escalate());
	CHECK(isIn<InProgressState>(*instance));
}

// ======================================================== instance work progress

TEST_CASE("Workflow instances start with independent work item progress") {
    WorkItem first("Review application");
    WorkItem second("Verify documents");

    WorkflowStage stage;
    stage.add(&first);
    stage.add(&second);

    WorkflowDefinition definition;
    definition.setRoot(&stage);

    WorkflowInstance a(&definition);
    WorkflowInstance b(&definition);

    REQUIRE(a.getWorkItemProgress().size() == 2);
    REQUIRE(b.getWorkItemProgress().size() == 2);

    CHECK_FALSE(a.isWorkItemComplete(&first));
    CHECK_FALSE(a.isWorkItemComplete(&second));
    CHECK_FALSE(b.isWorkItemComplete(&first));
    CHECK_FALSE(b.isWorkItemComplete(&second));
}

TEST_CASE("Completing work in one instance does not affect another") {
    WorkItem item("Review application");

    WorkflowStage stage;
    stage.add(&item);

    WorkflowDefinition definition;
    definition.setRoot(&stage);

    WorkflowInstance a(&definition);
    WorkflowInstance b(&definition);

    a.completeWorkItem(&item);

    CHECK(a.isWorkItemComplete(&item));
    CHECK_FALSE(b.isWorkItemComplete(&item));
}

TEST_CASE("WorkflowInstance rejects work items outside its definition") {
    WorkItem included("Review application");
    WorkItem external("Unrelated work");

    WorkflowStage stage;
    stage.add(&included);

    WorkflowDefinition definition;
    definition.setRoot(&stage);

    WorkflowInstance instance(&definition);

    CHECK_THROWS_AS(
        instance.completeWorkItem(&external),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        instance.isWorkItemComplete(&external),
        std::invalid_argument
    );
}

TEST_CASE("WorkflowInstance rejects null work items") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);

    CHECK_THROWS_AS(
        instance.completeWorkItem(nullptr),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        instance.isWorkItemComplete(nullptr),
        std::invalid_argument
    );
}
