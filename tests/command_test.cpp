#include "doctest.h"
#include "TestFixtures.h"
#include "engine/Invoker.h"

using namespace tividy;

TEST_CASE("Executing commands drives the instance and the Invoker logs them") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    inv.execute(new CompleteWorkCommand(*w, "intake", "alice", "done"));
    CHECK(w->findItem("review")->assignee() == "bob");
    REQUIRE(inv.log().size() == 2);
    CHECK(inv.log()[1] == "Complete intake by alice (done)");
}

TEST_CASE("Undo restores state, item progress and workloads (Memento)") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    inv.execute(new CompleteWorkCommand(*w, "intake", "alice", "done"));
    REQUIRE(w->findItem("intake")->status() == WorkStatus::Completed);
    REQUIRE(w->findParticipant("bob")->workload() == 1);

    inv.undoLast();
    CHECK(w->findItem("intake")->status() == WorkStatus::Assigned);
    CHECK(w->findItem("review")->status() == WorkStatus::Pending);
    CHECK(w->findParticipant("alice")->workload() == 1);
    CHECK(w->findParticipant("bob")->workload() == 0);
    CHECK(w->stateName() == "InProgress");

    inv.undoLast();  // undo the start too
    CHECK(w->stateName() == "Submitted");
    CHECK(w->findItem("intake")->status() == WorkStatus::Pending);
    CHECK_THROWS_AS(inv.undoLast(), WorkflowError);  // nothing left
}

TEST_CASE("Undo keeps the audit trail: history only grows") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    size_t before = w->history().events().size();
    inv.undoLast();
    CHECK(w->history().events().size() > before);
    CHECK(w->history().eventsOfType(EventType::Undone).size() == 1);
}

TEST_CASE("A rejected command changes nothing, is audited, and is not undoable") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    CHECK_THROWS_AS(inv.execute(new CompleteWorkCommand(*w, "intake", "bob", "done")), WorkflowError);
    CHECK(w->findItem("intake")->status() == WorkStatus::Assigned);
    CHECK(w->history().snapshotCount() == 1);  // only the successful start's snapshot remains
    REQUIRE(w->history().eventsOfType(EventType::InvalidAction).size() == 1);
    CHECK(inv.log().size() == 1);
}

TEST_CASE("A failed escalation (no handler available) leaves the instance exactly as it was") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    w->findParticipant("bob")->setAvailable(false);
    w->findParticipant("carol")->setAvailable(false);
    CHECK_THROWS_AS(inv.execute(new EscalateWorkCommand(*w, "alice", "help")), WorkflowError);
    CHECK(w->stateName() == "InProgress");
    CHECK(w->escalationCount() == 0);
}

TEST_CASE("Close is irreversible and blocks undo of earlier commands") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    inv.execute(new CloseWorkflowCommand(*w, "alice", "withdrawn"));
    CHECK_FALSE(inv.canUndo());
    CHECK_THROWS_AS(inv.undoLast(), WorkflowError);
    CHECK(w->stateName() == "Closed");
}

TEST_CASE("Escalate then approve, and undo of the escalation returns to InProgress") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    Invoker inv;
    inv.execute(new StartWorkflowCommand(*w, "system"));
    inv.execute(new EscalateWorkCommand(*w, "alice", "customer is upset"));
    CHECK(w->stateName() == "Escalated");
    inv.undoLast();
    CHECK(w->stateName() == "InProgress");
    CHECK(w->escalatedTo().empty());
    CHECK(w->escalationCount() == 0);

    inv.execute(new EscalateWorkCommand(*w, "alice", "customer is upset"));
    inv.execute(new ApproveWorkCommand(*w, "bob", "handled"));
    CHECK(w->stateName() == "InProgress");
}

TEST_CASE("A command cannot be executed twice") {
    std::unique_ptr<WorkflowInstance> w = fixtures::newInstance("WF-1", fixtures::approvalDefinition());
    StartWorkflowCommand cmd(*w, "system");
    cmd.execute();
    CHECK_THROWS_AS(cmd.execute(), WorkflowError);
}
