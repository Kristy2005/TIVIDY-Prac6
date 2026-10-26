#include "doctest.h"
#include <string>
#include <vector>

#include "engine/PriorityDecorator.h"
#include "engine/SubProcess.h"
#include "engine/WorkItem.h"
#include "engine/WorkflowStage.h"

namespace {

/** Test double that records execution and lets a test control completion. */
class RecordingComponent : public WorkflowComponent {
public:
    explicit RecordingComponent(const std::string& label, std::vector<std::string>* executionOrder = nullptr) : label(label), executionOrder(executionOrder), calls(0), complete(false) {}

    virtual void execute() {
        ++calls;
        if (executionOrder != nullptr) {
            executionOrder->push_back(label);
        }
    }

    virtual bool isComplete() const {
        return complete;
    }

    void markComplete() {
        complete = true;
    }

    std::string label;
    std::vector<std::string>* executionOrder;
    int calls;
    bool complete;
};

class DestructionTrackedComponent : public WorkflowComponent {
public:
    explicit DestructionTrackedComponent(int* destructionCount)
        : destructionCount(destructionCount) {
    }

    virtual ~DestructionTrackedComponent() {
        ++(*destructionCount);
    }

    virtual void execute() {
    }

    virtual bool isComplete() const {
        return false;
    }

private:
    int* destructionCount;
};

}

TEST_CASE("WorkItem starts incomplete and complete() marks it complete") {
    WorkItem item("Review complaint");

    CHECK(item.getName() == "Review complaint");
    CHECK_FALSE(item.isComplete());
    item.complete();
    CHECK(item.isComplete());

    item.complete();
    CHECK(item.isComplete());
}

TEST_CASE("Default WorkItem has an empty name and starts incomplete") {
    WorkItem item;

    CHECK(item.getName().empty());
    CHECK_FALSE(item.isComplete());
}

TEST_CASE("WorkItem execution does not complete the real-world task") {
    WorkItem item("Investigate complaint");

    item.execute();

    CHECK_FALSE(item.isComplete());
    item.complete();
    CHECK(item.isComplete());
}

TEST_CASE("WorkflowStage executes children in insertion order and tracks completion") {
    std::vector<std::string> order;
    WorkflowStage stage;
    RecordingComponent first("first", &order);
    RecordingComponent second("second", &order);

    stage.add(&first);
    stage.add(&second);

    CHECK_FALSE(stage.isComplete());
    stage.execute();
    REQUIRE(order.size() == 2);
    CHECK(order[0] == "first");
    CHECK(order[1] == "second");
    CHECK(first.calls == 1);
    CHECK(second.calls == 1);
    CHECK_FALSE(stage.isComplete());

    first.markComplete();
    CHECK_FALSE(stage.isComplete());
    second.markComplete();
    CHECK(stage.isComplete());
}

TEST_CASE("WorkflowStage ignores null and duplicate children and supports removal") {
    WorkflowStage stage;
    RecordingComponent child("child");

    stage.add(nullptr);
    stage.add(&child);
    stage.add(&child);
    stage.execute();
    CHECK(child.calls == 1);

    stage.remove(&child);
    stage.execute();
    CHECK(child.calls == 1);
    CHECK(stage.isComplete());

    stage.remove(&child);
    stage.remove(nullptr);
}

TEST_CASE("An empty WorkflowStage is complete") {
    WorkflowStage stage;
    CHECK(stage.isComplete());
}

TEST_CASE("SubProcess can contain and execute nested workflow components") {
    SubProcess nested;
    WorkflowStage stage;
    SubProcess process;
    RecordingComponent child("child");

    nested.add(&child);
    stage.add(&nested);
    process.add(&stage);

    CHECK_FALSE(process.isComplete());
    process.execute();
    CHECK(child.calls == 1);
    CHECK_FALSE(process.isComplete());

    child.markComplete();
    CHECK(process.isComplete());
}

TEST_CASE("SubProcess ignores null and duplicate children and supports removal") {
    SubProcess process;
    RecordingComponent child("child");

    process.add(nullptr);
    process.add(&child);
    process.add(&child);
    process.execute();
    CHECK(child.calls == 1);

    process.remove(&child);
    process.execute();
    CHECK(child.calls == 1);
    CHECK(process.isComplete());
    process.remove(nullptr);
}

TEST_CASE("Composite containers reject direct self-reference") {
    WorkflowStage stage;
    SubProcess process;

    stage.add(&stage);
    process.add(&process);

    CHECK(stage.isComplete());
    CHECK(process.isComplete());
    CHECK_NOTHROW(stage.execute());
    CHECK_NOTHROW(process.execute());
}

TEST_CASE("WorkflowStage does not own or delete its child components") {
    int destructionCount = 0;
    DestructionTrackedComponent* child =
        new DestructionTrackedComponent(&destructionCount);

    {
        WorkflowStage stage;
        stage.add(child);
    }

    CHECK(destructionCount == 0);
    delete child;
    CHECK(destructionCount == 1);
}

TEST_CASE("WorkflowStage can contain and track a PriorityDecorator") {
    WorkItem item("Urgent complaint");
    PriorityDecorator urgent(&item, 50);
    WorkflowStage stage;

    stage.add(&urgent);
    CHECK_FALSE(stage.isComplete());
    stage.execute();
    CHECK_FALSE(item.isComplete());

    item.complete();
    CHECK(stage.isComplete());
}
