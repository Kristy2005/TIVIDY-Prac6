#include "doctest.h"
#include <stdexcept>
#include "engine/PriorityDecorator.h"
#include "engine/WorkItem.h"

using namespace tividy;

namespace {
template <typename T>
struct IsAbstract {
    template <typename U>
    static char test(decltype(U(static_cast<tividy::WorkflowComponent*>(0)))*);

    template <typename U>
    static long test(...);

    enum { value = (sizeof(test<T>(0)) == sizeof(long)) };
};

class RecordingComponent : public WorkflowComponent {
public:
    RecordingComponent() : calls(0), complete(false) {
    }

    virtual void execute() {
        ++calls;
    }

    virtual bool isComplete() const {
        return complete;
    }

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

TEST_CASE("WorkItemDecorator is an abstract base class") {
    CHECK(IsAbstract<WorkItemDecorator>::value);
}

TEST_CASE("PriorityDecorator exposes priority and delegates execution") {
    RecordingComponent component;
    PriorityDecorator decorated(&component, 10);

    CHECK(decorated.getPriority() == 10);
    CHECK(component.calls == 0);
    CHECK_FALSE(decorated.isComplete());

    decorated.execute();
    CHECK(component.calls == 1);
    CHECK_FALSE(decorated.isComplete());

    component.complete = true;
    CHECK(decorated.isComplete());
}

TEST_CASE("PriorityDecorator preserves WorkItem completion semantics") {
    WorkItem item("Review urgent complaint");
    PriorityDecorator decorated(&item, 100);

    CHECK_FALSE(decorated.isComplete());
    decorated.execute();
    CHECK_FALSE(item.isComplete());
    CHECK_FALSE(decorated.isComplete());

    item.complete();
    CHECK(decorated.isComplete());
    CHECK(decorated.getPriority() == 100);
}

TEST_CASE("PriorityDecorator rejects a null wrapped component") {
    CHECK_THROWS_AS(PriorityDecorator(nullptr, 5), std::invalid_argument);
}

TEST_CASE("PriorityDecorator stores the supplied integer priority") {
    WorkItem item("Some work");
    PriorityDecorator low(&item, -1);
    PriorityDecorator high(&item, 999);

    CHECK(low.getPriority() == -1);
    CHECK(high.getPriority() == 999);
}

TEST_CASE("PriorityDecorator does not own or delete its wrapped component") {
    int destructionCount = 0;
    DestructionTrackedComponent* component =
        new DestructionTrackedComponent(&destructionCount);

    {
        PriorityDecorator decorated(component, 1);
    }

    CHECK(destructionCount == 0);
    delete component;
    CHECK(destructionCount == 1);
}