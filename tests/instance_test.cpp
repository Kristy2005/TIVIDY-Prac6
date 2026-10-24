#include "doctest.h"
#include "TestFixtures.h"

using namespace tividy;

TEST_CASE("Lifecycle: Submitted -> Active/InProgress on start, work is auto-assigned by the strategy") {
    std::shared_ptr<WorkflowDefinition> def = fixtures::approvalDefinition();
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", def);
    CHECK(w->stateName() == "Submitted");
    CHECK(w->findItem("intake")->status() == WorkStatus::Pending);

    w->start("system");
    CHECK(w->stateName() == "InProgress");
    CHECK(w->findItem("intake")->status() == WorkStatus::Assigned);
    CHECK(w->findItem("intake")->assignee() == "alice");
    CHECK(w->findItem("review")->status() == WorkStatus::Pending);  // waits for intake
    CHECK(w->findParticipant("alice")->workload() == 1);
}

TEST_CASE("Decision branch: approval takes the approve path and skips the reject path") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->complete("intake", "alice", "done");
    CHECK(w->findItem("review")->assignee() == "bob");
    w->complete("review", "bob", "approved");

    CHECK(w->findItem("fulfil")->status() == WorkStatus::Assigned);
    CHECK(w->findItem("reject_notice")->status() == WorkStatus::Skipped);
    CHECK(w->findItem("archive")->status() == WorkStatus::Pending);  // join waits for fulfil
    CHECK(w->history().eventsOfType(EventType::RoutingDecision).size() == 2);

    w->complete("fulfil", "alice", "done");
    CHECK(w->findItem("archive")->status() == WorkStatus::Assigned);
    w->complete("archive", "alice", "done");
    CHECK(w->stateName() == "Resolved");
    CHECK(w->progress().percent() == 100);
}

TEST_CASE("Decision branch: rejection takes the exception path and still completes") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->complete("intake", "alice", "done");
    w->complete("review", "bob", "rejected");
    CHECK(w->findItem("fulfil")->status() == WorkStatus::Skipped);
    CHECK(w->findItem("reject_notice")->status() == WorkStatus::Assigned);
    w->complete("reject_notice", "alice", "done");
    w->complete("archive", "alice", "done");
    CHECK(w->stateName() == "Resolved");
}

TEST_CASE("Invalid operations are rejected with InvalidTransition and change nothing") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    CHECK_THROWS_AS(w->complete("intake", "alice", "done"), InvalidTransition);  // not started
    CHECK_THROWS_AS(w->escalate("alice", "too early"), InvalidTransition);
    w->start("system");
    CHECK_THROWS_AS(w->start("system"), InvalidTransition);                      // already started
    CHECK_THROWS_AS(w->complete("review", "bob", "approved"), InvalidTransition); // still Pending
    CHECK_THROWS_AS(w->complete("intake", "bob", "done"), WorkflowError);         // not the assignee
    CHECK_THROWS_AS(w->complete("nonsense", "alice", "done"), WorkflowError);
    CHECK(w->findItem("intake")->status() == WorkStatus::Assigned);
}

TEST_CASE("Manual assignment validates role, availability and re-assignment frees the old owner") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->addParticipant("dave", "clerk");
    w->start("system");
    CHECK_THROWS_AS(w->assign("intake", "bob", "alice"), WorkflowError);  // bob is a reviewer
    w->assign("intake", "dave", "alice");
    CHECK(w->findItem("intake")->assignee() == "dave");
    CHECK(w->findParticipant("alice")->workload() == 0);
    CHECK(w->findParticipant("dave")->workload() == 1);
}

TEST_CASE("Escalation: handler is chosen by the chain, work freezes, only the handler can resolve") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->complete("intake", "alice", "done");

    w->escalate("bob", "Needs a second opinion");
    CHECK(w->stateName() == "Escalated");
    CHECK(w->escalatedTo() == "bob");  // level 1 -> ApprovalHandler(reviewer)
    CHECK_THROWS_AS(w->complete("review", "bob", "approved"), InvalidTransition);  // frozen
    CHECK_THROWS_AS(w->resolveEscalation("alice", "nope"), WorkflowError);

    w->resolveEscalation("bob", "Looks fine");
    CHECK(w->stateName() == "InProgress");
    w->escalate("bob", "Still unsure");                 // second escalation needs level 2
    CHECK(w->escalatedTo() == "carol");                 // -> FinalDecisionHandler(manager)
    CHECK(w->escalationCount() == 2);
}

TEST_CASE("Escalation fails cleanly when nobody can take it") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->findParticipant("bob")->setAvailable(false);
    w->findParticipant("carol")->setAvailable(false);
    CHECK_THROWS_AS(w->escalate("alice", "help"), WorkflowError);
    CHECK(w->stateName() == "InProgress");
}

TEST_CASE("Cancelling an item cancels everything that depends on it") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->cancelItem("intake", "alice", "duplicate request");
    CHECK(w->findItem("intake")->status() == WorkStatus::Cancelled);
    CHECK(w->findItem("review")->status() == WorkStatus::Cancelled);
    CHECK(w->findItem("archive")->status() == WorkStatus::Cancelled);
    CHECK(w->stateName() == "Resolved");  // nothing left to do
    CHECK(w->findParticipant("alice")->workload() == 0);
}

TEST_CASE("Closing cancels open work and closes the door on everything else") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    w->start("system");
    w->close("alice", "Requester withdrew");
    CHECK(w->stateName() == "Closed");
    CHECK(w->findItem("intake")->status() == WorkStatus::Cancelled);
    CHECK(w->findParticipant("alice")->workload() == 0);
    CHECK_THROWS_AS(w->assign("intake", "alice", "alice"), InvalidTransition);
    CHECK_THROWS_AS(w->close("alice", "again"), InvalidTransition);
    CHECK(w->history().eventsOfType(EventType::WorkflowClosed).size() == 1);
}

TEST_CASE("Instances from one definition are fully isolated from each other") {
    std::shared_ptr<WorkflowDefinition> def = fixtures::approvalDefinition();
    std::unique_ptr<WorkflowInstance> a = fixtures::newInstance("WF-A", def);
    std::unique_ptr<WorkflowInstance> b = fixtures::newInstance("WF-B", def);
    a->start("system");
    a->complete("intake", "alice", "done");
    a->escalate("bob", "check");
    a->setData("customer", "A-corp");

    CHECK(b->stateName() == "Submitted");
    CHECK(b->findItem("intake")->status() == WorkStatus::Pending);
    CHECK(b->findParticipant("alice")->workload() == 0);
    CHECK(b->escalationCount() == 0);
    CHECK(b->getData("customer") == "");
    CHECK(b->history().events().size() == 1);   // just "created"
    CHECK(def->prototype().find("intake")->status() == WorkStatus::Pending);
}

TEST_CASE("Participants must hold a role the definition knows") {
    WorkflowInstance w("WF-1", fixtures::approvalDefinition());
    CHECK_THROWS_AS(w.addParticipant("zed", "astronaut"), WorkflowError);
    w.addParticipant("zed", "clerk");
    CHECK_THROWS_AS(w.addParticipant("zed", "clerk"), WorkflowError);  // duplicate name
}

TEST_CASE("Urgent work is listed before ordinary work") {
    ConcreteWorkflowBuilder b("Two independent items");
    b.addRole("clerk");
    b.addWorkItem("", WorkItemSpec("routine", "Routine task", "clerk"));
    b.addWorkItem("", WorkItemSpec("hot", "Hot task", "clerk", Priority::Urgent));
    WorkflowInstance w("WF-1", b.build());
    w.addParticipant("alice", "clerk");
    w.start("system");
    std::vector<WorkflowComponent*> work = w.availableWork();
    REQUIRE(work.size() == 2);
    CHECK(work[0]->id() == "hot");      // defined second, listed first
    CHECK(work[1]->id() == "routine");
}
