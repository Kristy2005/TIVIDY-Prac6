#include <memory>
#include "doctest.h"
#include "engine/WorkItem.h"
#include "engine/WorkflowStage.h"

using namespace tividy;

namespace {
WorkflowStage* sampleTree() {
    WorkflowStage* root = new WorkflowStage("root", "Root");
    WorkflowStage* s1 = new WorkflowStage("s1", "Stage 1");
    s1->add(new WorkItem("a", "Item A", "clerk"));
    SubProcess* sub = new SubProcess("sub", "Nested");
    sub->add(new WorkItem("b", "Item B", "clerk"));
    s1->add(sub);
    root->add(s1);
    root->add(new WorkItem("c", "Item C", "clerk"));
    return root;
}
}

TEST_CASE("Composite finds nodes and lists leaves in definition order") {
    std::unique_ptr<WorkflowStage> root(sampleTree());
    std::vector<WorkflowComponent*> leaves;
    root->collectLeaves(leaves);
    REQUIRE(leaves.size() == 3);
    CHECK(leaves[0]->id() == "a");
    CHECK(leaves[1]->id() == "b");
    CHECK(leaves[2]->id() == "c");
    CHECK(root->find("sub") != 0);
    CHECK(root->find("nope") == 0);
    CHECK(root->find("sub")->kind() == "SubProcess");
}

TEST_CASE("Prototype: a cloned tree is independent of the original") {
    std::unique_ptr<WorkflowStage> original(sampleTree());
    std::unique_ptr<WorkflowComponent> copy(original->clone());

    copy->find("a")->makeAvailable();
    CHECK(copy->find("a")->status() == WorkStatus::Available);
    CHECK(original->find("a")->status() == WorkStatus::Pending);
    CHECK(copy->find("sub")->kind() == "SubProcess");  // subclass type survives cloning
}

TEST_CASE("WorkItem lifecycle allows legal moves and rejects illegal ones") {
    WorkItem w("w", "Work", "clerk");
    CHECK_THROWS_AS(w.assign("alice"), InvalidTransition);   // still Pending
    CHECK_THROWS_AS(w.complete("done"), InvalidTransition);
    w.makeAvailable();
    w.assign("alice");
    CHECK(w.assignee() == "alice");
    w.complete("approved");
    CHECK(w.status() == WorkStatus::Completed);
    CHECK(w.outcome() == "approved");
    CHECK_THROWS_AS(w.cancel(), InvalidTransition);          // already terminal
}

TEST_CASE("Composite status is derived from its leaves") {
    std::unique_ptr<WorkflowStage> root(sampleTree());
    CHECK(root->status() == WorkStatus::Pending);
    root->find("a")->makeAvailable();
    CHECK(root->status() == WorkStatus::Available);
    root->find("a")->assign("alice");
    CHECK(root->status() == WorkStatus::Assigned);
}

TEST_CASE("Leaf-only operations on a composite are rejected") {
    WorkflowStage s("s", "Stage");
    CHECK_THROWS_AS(s.assign("alice"), WorkflowError);
    WorkItem w("w", "Work", "clerk");
    CHECK_THROWS_AS(w.add(new WorkItem("x", "X", "clerk")), WorkflowError);
}

TEST_CASE("A routing condition adds an implicit dependency") {
    WorkItem w("w", "Work", "clerk");
    w.setCondition("decision", "approved");
    CHECK(w.hasCondition());
    REQUIRE(w.dependencies().size() == 1);
    CHECK(w.dependencies()[0] == "decision");
}
