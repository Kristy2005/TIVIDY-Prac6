#include "doctest.h"
#include "TestFixtures.h"
#include "engine/EmailAdapter.h"
#include "engine/TIVIDYFacade.h"

using namespace tividy;

namespace {
struct World {
    ExternalEmailService gateway;
    EmailAdapter adapter;
    TIVIDYFacade app;
    World() : adapter(gateway), app(&adapter) { app.registerWorkflow(fixtures::approvalDefinition()); }

    std::string newCase() {
        std::string id = app.createInstance("Request approval");
        app.addParticipant(id, "alice", "clerk");
        app.addParticipant(id, "bob", "reviewer");
        app.addParticipant(id, "carol", "manager");
        return id;
    }
};
}

TEST_CASE("Facade runs a whole case end to end through one interface") {
    World w;
    std::string id = w.newCase();
    w.app.startWorkflow(id);
    w.app.completeWork(id, "intake", "alice");
    w.app.completeWork(id, "review", "bob", "approved");
    w.app.completeWork(id, "fulfil", "alice");
    w.app.completeWork(id, "archive", "alice");
    CHECK(w.app.instance(id).stateName() == "Resolved");
    CHECK(w.app.statusReport(id).find("Resolved") != std::string::npos);
    CHECK(w.app.historyReport(id).find("ROUTING_DECISION") != std::string::npos);
}

TEST_CASE("Facade instances are independent of one another") {
    World w;
    std::string a = w.newCase();
    std::string b = w.newCase();
    CHECK(a != b);
    w.app.startWorkflow(a);
    w.app.completeWork(a, "intake", "alice");
    w.app.closeWorkflow(a, "alice", "withdrawn");
    CHECK(w.app.instance(a).stateName() == "Closed");
    CHECK(w.app.instance(b).stateName() == "Submitted");
    w.app.startWorkflow(b);  // b is unaffected by a's closure
    CHECK(w.app.instance(b).findItem("intake")->status() == WorkStatus::Assigned);
}

TEST_CASE("Assignments and escalations are emailed through the Adapter") {
    World w;
    std::string id = w.newCase();
    w.app.startWorkflow(id);
    REQUIRE(w.gateway.outbox().size() == 1);               // intake -> alice
    CHECK(w.gateway.outbox()[0].address == "alice@tividy.example");
    w.app.completeWork(id, "intake", "alice");             // review -> bob
    w.app.escalateWork(id, "bob", "second opinion");       // escalated to bob
    CHECK(w.gateway.outbox().size() == 3);
    CHECK(w.gateway.outbox()[2].raw.find("ESCALATED") != std::string::npos);
}

TEST_CASE("A dead email gateway is logged but never blocks the workflow") {
    World w;
    std::string id = w.newCase();
    w.gateway.setOnline(false);
    CHECK_NOTHROW(w.app.startWorkflow(id));
    CHECK(w.app.instance(id).stateName() == "InProgress");
    CHECK(w.app.instance(id).history().eventsOfType(EventType::NotificationFailed).size() == 1);
    w.app.completeWork(id, "intake", "alice");
    w.app.escalateWork(id, "bob", "x");
    CHECK(w.app.instance(id).stateName() == "Escalated");  // the escalation still stands
}

TEST_CASE("Facade errors: unknown ids and invalid actions") {
    World w;
    CHECK_THROWS_AS(w.app.startWorkflow("WF-999"), WorkflowError);
    CHECK_THROWS_AS(w.app.createInstance("No such workflow"), WorkflowError);
    std::string id = w.newCase();
    CHECK_THROWS_AS(w.app.completeWork(id, "intake", "alice"), InvalidTransition);  // not started
    CHECK_FALSE(w.app.canUndo(id));
    w.app.startWorkflow(id);
    CHECK(w.app.canUndo(id));
    w.app.undo(id);
    CHECK(w.app.instance(id).stateName() == "Submitted");
}

TEST_CASE("The same engine works with no notifier at all") {
    TIVIDYFacade app;
    app.registerWorkflow(fixtures::approvalDefinition());
    std::string id = app.createInstance("Request approval");
    app.addParticipant(id, "alice", "clerk");
    CHECK_NOTHROW(app.startWorkflow(id));
    CHECK(app.workflowNames().size() == 1);
}
