#include <memory>
#include "doctest.h"
#include "engine/WorkItem.h"
#include "engine/WorkItemDecorator.h"
#include "engine/WorkflowStage.h"

using namespace tividy;

TEST_CASE("PriorityDecorator exposes priority and forwards the item interface") {
    PriorityDecorator d(new WorkItem("w", "Check exam clash", "clerk"), Priority::Urgent);
    CHECK(d.id() == "w");
    CHECK(d.priority() == Priority::Urgent);
    CHECK(d.displayName() == "[Urgent] Check exam clash");
    CHECK(d.requiredRole() == "clerk");
    d.makeAvailable();
    d.assign("alice");
    CHECK(d.status() == WorkStatus::Assigned);
    CHECK(d.assignee() == "alice");
}

TEST_CASE("Decorated items stay reachable and keep their wrapper when the tree is cloned") {
    WorkflowStage root("root", "Root");
    root.add(new PriorityDecorator(new WorkItem("w", "Work", "clerk"), Priority::High));
    std::unique_ptr<WorkflowComponent> copy(root.clone());

    std::vector<WorkflowComponent*> leaves;
    copy->collectLeaves(leaves);
    REQUIRE(leaves.size() == 1);
    CHECK(leaves[0]->priority() == Priority::High);

    leaves[0]->makeAvailable();
    CHECK(root.find("w")->status() == WorkStatus::Pending);  // original untouched
}

TEST_CASE("A decorator needs something to wrap") {
    CHECK_THROWS_AS(PriorityDecorator(0, Priority::High), WorkflowError);
}
