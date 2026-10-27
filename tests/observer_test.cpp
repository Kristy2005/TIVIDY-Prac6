#include "doctest.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "engine/NotificationService.h"
#include "engine/EmailAdapter.h"
#include "engine/ExternalEmailService.h"
#include "engine/NotificationObserver.h"
#include "engine/ProgressObserver.h"
#include "engine/Participant.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowObserver.h"
#include "engine/WorkflowState.h"

using namespace tividy;

/*
 * Observer pattern tests.
 *
 * Section 1: structure and polymorphism.
 * Section 2: EmailAdapter and ExternalEmailService. EmailAdapter requires
 *            both a recipient and a configured service: a missing one is a
 *            caller error and throws rather than silently doing nothing
 *            (see EmailAdapter.h).
 * Section 3: ProgressObserver and NotificationObserver.
 * Section 4: observers working with WorkflowInstance.
 */

namespace {

/**
 * Result of running an action while std::cout is redirected.
 * threw is true if the action threw. text is everything it printed.
 */
struct CapturedRun {
    bool threw;
    std::string text;
};

/**
 * Runs @p action with std::cout redirected and returns what it printed.
 *
 * doctest writes failure messages to std::cout, so no CHECK may run while
 * the redirect is active: its message would be swallowed. Callers therefore
 * run the action here and assert on the returned result afterwards.
 */
template <typename Action>
CapturedRun runCapturingCout(Action action) {
    std::stringstream buffer;
    std::streambuf* oldBuffer = std::cout.rdbuf(buffer.rdbuf());
    CapturedRun result;
    result.threw = false;
    try {
        action();
    } catch (...) {
        result.threw = true;
    }
    std::cout.rdbuf(oldBuffer);
    result.text = buffer.str();
    return result;
}

/** Counts how many times @p needle appears in @p haystack. */
int countOccurrences(const std::string& haystack, const std::string& needle) {
    int count = 0;
    std::string::size_type pos = haystack.find(needle);
    while (pos != std::string::npos) {
        ++count;
        pos = haystack.find(needle, pos + needle.size());
    }
    return count;
}

/**
 * Returns a non-null pointer to use as a WorkflowInstance* identity token in
 * tests that never dereference it. Observers in these tests only compare or
 * store the pointer.
 */
WorkflowInstance* dummyInstance() {
    static char token = 0;
    return reinterpret_cast<WorkflowInstance*>(&token);
}

/** Minimal concrete WorkflowState. Its transitions do nothing. */
class StubState : public WorkflowState {
public:
    void activate(WorkflowInstance*) override {}
    void startProcessing(WorkflowInstance*) override {}
    void resolve(WorkflowInstance*) override {}
    void close(WorkflowInstance*) override {}
};

/**
 * Test double for WorkflowObserver. Records every notification.
 * When readState is true, it also reads the instance's current state during
 * update(), which lets tests check that observers see the new state.
 */
class RecordingObserver : public WorkflowObserver {
public:
    explicit RecordingObserver(const std::string& label = std::string(),
                               std::vector<std::string>* log = nullptr)
        : label(label), log(log), updateCount(0), lastInstance(nullptr),
          readState(false), stateSeen(nullptr) {}

    void update(WorkflowInstance* instance) override {
        ++updateCount;
        lastInstance = instance;
        if (readState && instance != nullptr) {
            stateSeen = instance->getState();
        }
        if (log != nullptr) {
            log->push_back(label);
        }
    }

    std::string label;
    std::vector<std::string>* log;
    int updateCount;
    WorkflowInstance* lastInstance;
    bool readState;
    WorkflowState* stateSeen;
};

} // namespace

// ---------------------------------------------------------------------------
// Section 1: structure and polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("ProgressObserver and NotificationObserver implement WorkflowObserver") {
    CHECK(std::is_base_of<WorkflowObserver, ProgressObserver>::value);
    CHECK(std::is_base_of<WorkflowObserver, NotificationObserver>::value);
}

TEST_CASE("WorkflowObserver is abstract and cannot be created directly") {
    CHECK(std::is_abstract<WorkflowObserver>::value);
}

TEST_CASE("NotificationService is abstract and EmailAdapter implements it") {
    CHECK(std::is_abstract<NotificationService>::value);
    CHECK(std::is_base_of<NotificationService, EmailAdapter>::value);
    CHECK_FALSE(std::is_abstract<EmailAdapter>::value);
}

TEST_CASE("EmailAdapter starts with no email service") {
    EmailAdapter adapter;
    CHECK(adapter.service == nullptr);
}

TEST_CASE("Observer receives the instance it is notified about through the base pointer") {
    RecordingObserver observer;
    WorkflowObserver* base = &observer;
    WorkflowInstance* instance = dummyInstance();

    base->update(instance);

    CHECK(observer.updateCount == 1);
    CHECK(observer.lastInstance == instance);
}

// ---------------------------------------------------------------------------
// Section 2: EmailAdapter and ExternalEmailService
// ---------------------------------------------------------------------------

TEST_CASE("ExternalEmailService prints the address and body") {
    ExternalEmailService service;

    CapturedRun run = runCapturingCout([&] {
        service.sendEmail("thandi@example.com", "Your case was resolved.");
    });

    CHECK_FALSE(run.threw);
    CHECK(run.text.find("thandi@example.com") != std::string::npos);
    CHECK(run.text.find("Your case was resolved.") != std::string::npos);
}

TEST_CASE("EmailAdapter sends the message through the email service") {
    ExternalEmailService service;
    EmailAdapter adapter;
    adapter.service = &service;
    Participant recipient("Thandi Dlamini");

    CapturedRun run = runCapturingCout([&] {
        adapter.sendNotification(&recipient, "Your case was resolved.");
    });

    // Participant has no email address, so the recipient's name is used.
    CHECK_FALSE(run.threw);
    CHECK(run.text.find("Thandi Dlamini") != std::string::npos);
    CHECK(run.text.find("Your case was resolved.") != std::string::npos);
}

TEST_CASE("EmailAdapter throws when there is no email service") {
    EmailAdapter adapter;
    Participant recipient("Thandi Dlamini");

    CHECK_THROWS_AS(
        adapter.sendNotification(&recipient, "Your case was resolved."),
        std::logic_error);
}

TEST_CASE("EmailAdapter throws when the recipient is null") {
    ExternalEmailService service;
    EmailAdapter adapter;
    adapter.service = &service;

    CHECK_THROWS_AS(
        adapter.sendNotification(nullptr, "Your case was resolved."),
        std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Section 3: concrete observers
// ---------------------------------------------------------------------------

TEST_CASE("ProgressObserver reports that the workflow state changed") {
    ProgressObserver observer;

    CapturedRun run = runCapturingCout([&] { observer.update(dummyInstance()); });

    CHECK_FALSE(run.threw);
    CHECK(run.text.find("Workflow state changed.") != std::string::npos);
}

TEST_CASE("ProgressObserver prints exactly one message per notification") {
    ProgressObserver observer;

    CapturedRun run = runCapturingCout([&] {
        observer.update(dummyInstance());
        observer.update(dummyInstance());
    });

    CHECK_FALSE(run.threw);
    CHECK(countOccurrences(run.text, "Workflow state changed.") == 2);
}

TEST_CASE("NotificationObserver accepts an update without throwing") {
    NotificationObserver observer;
    CHECK_NOTHROW(observer.update(dummyInstance()));
}

// ---------------------------------------------------------------------------
// Section 4: observers working with WorkflowInstance
// ---------------------------------------------------------------------------

TEST_CASE("Observer is not notified before any state change") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    instance.attach(&observer);

    CHECK(observer.updateCount == 0);
}

TEST_CASE("Attached observer is notified when the state changes") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    StubState state;
    instance.attach(&observer);

    instance.setState(&state);

    CHECK(observer.updateCount == 1);
}

TEST_CASE("Each state change notifies the observer once") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    StubState first;
    StubState second;
    instance.attach(&observer);

    instance.setState(&first);
    instance.setState(&second);

    CHECK(observer.updateCount == 2);
}

TEST_CASE("Detached observer is no longer notified") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    StubState state;
    instance.attach(&observer);
    instance.detach(&observer);

    instance.setState(&state);

    CHECK(observer.updateCount == 0);
}

TEST_CASE("Observer is notified with the instance that changed") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    StubState state;
    instance.attach(&observer);

    instance.setState(&state);

    CHECK(observer.lastInstance == &instance);
}

TEST_CASE("Every attached observer is notified about the same instance") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver first;
    RecordingObserver second;
    StubState state;
    instance.attach(&first);
    instance.attach(&second);

    instance.setState(&state);

    CHECK(first.lastInstance == &instance);
    CHECK(second.lastInstance == &instance);
}

TEST_CASE("Observers are notified in the order they were attached") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    std::vector<std::string> log;
    RecordingObserver first("first", &log);
    RecordingObserver second("second", &log);
    StubState state;
    instance.attach(&first);
    instance.attach(&second);

    instance.setState(&state);

    REQUIRE(log.size() == 2);
    CHECK(log[0] == "first");
    CHECK(log[1] == "second");
}

TEST_CASE("Observer sees the new state when it is notified") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    RecordingObserver observer;
    observer.readState = true;
    StubState state;
    instance.attach(&observer);

    instance.setState(&state);

    CHECK(observer.stateSeen == &state);
}

TEST_CASE("Observer is not notified about a different instance") {
    WorkflowDefinition definition;
    WorkflowInstance watched(&definition);
    WorkflowInstance other(&definition);
    RecordingObserver observer;
    StubState state;
    watched.attach(&observer);

    other.setState(&state);

    CHECK(observer.updateCount == 0);
}

TEST_CASE("ProgressObserver reports a state change of an attached instance") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    ProgressObserver observer;
    StubState state;
    instance.attach(&observer);

    CapturedRun run = runCapturingCout([&] { instance.setState(&state); });

    CHECK_FALSE(run.threw);
    CHECK(run.text.find("Workflow state changed.") != std::string::npos);
}