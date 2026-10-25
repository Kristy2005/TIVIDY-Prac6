#include "doctest.h"
#include "engine/EmailAdapter.h"
#include "engine/NotificationObserver.h"
#include "engine/WorkflowObserver.h"

using namespace tividy;

namespace {
struct Recorder : WorkflowObserver {
    std::vector<WorkflowEvent> seen;
    void update(const WorkflowEvent& e) { seen.push_back(e); }
};
struct TestSubject : WorkflowSubject {
    void fire(EventType t) { WorkflowEvent e; e.type = t; notify(e); }
};
WorkflowEvent assignedTo(const std::string& who) {
    WorkflowEvent e;
    e.type = EventType::WorkAssigned;
    e.instanceId = "WF-1";
    e.recipient = who;
    e.message = "You have new work";
    return e;
}
}

TEST_CASE("Subject notifies every attached observer in order, and respects detach") {
    TestSubject s;
    Recorder a, b;
    s.attach(&a);
    s.attach(&b);
    s.attach(&a);  // duplicate ignored
    CHECK(s.observerCount() == 2);
    s.fire(EventType::WorkAssigned);
    CHECK(a.seen.size() == 1);
    CHECK(b.seen.size() == 1);
    s.detach(&a);
    s.fire(EventType::WorkCompleted);
    CHECK(a.seen.size() == 1);
    CHECK(b.seen.size() == 2);
}

TEST_CASE("EmailAdapter translates a notification into the gateway's format") {
    ExternalEmailService gateway;
    EmailAdapter adapter(gateway, "uni.example");
    CHECK(adapter.send("alice", "Hello", "Body text"));
    REQUIRE(gateway.outbox().size() == 1);
    CHECK(gateway.outbox()[0].address == "alice@uni.example");
    CHECK(gateway.outbox()[0].raw == "Subject: Hello\n\nBody text");
    CHECK(adapter.send("bob@elsewhere.org", "Hi", "x"));
    CHECK(gateway.outbox()[1].address == "bob@elsewhere.org");
}

TEST_CASE("NotificationObserver only sends events that name a recipient") {
    ExternalEmailService gateway;
    EmailAdapter adapter(gateway);
    NotificationObserver obs(adapter);
    WorkflowEvent quiet;
    quiet.type = EventType::StateChanged;
    obs.update(quiet);
    CHECK(gateway.outbox().empty());
    obs.update(assignedTo("alice"));
    CHECK(gateway.outbox().size() == 1);
}

TEST_CASE("A failing gateway is reported to the sink but does not throw") {
    ExternalEmailService gateway;
    gateway.setOnline(false);
    EmailAdapter adapter(gateway);
    Recorder sink;
    NotificationObserver obs(adapter, &sink);
    CHECK_NOTHROW(obs.update(assignedTo("alice")));
    CHECK(obs.failureCount() == 1);
    REQUIRE(sink.seen.size() == 1);
    CHECK(sink.seen[0].type == EventType::NotificationFailed);
}
