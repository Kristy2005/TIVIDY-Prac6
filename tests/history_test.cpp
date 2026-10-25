#include "doctest.h"
#include "engine/WorkflowHistory.h"

using namespace tividy;

TEST_CASE("History numbers events in order and filters by type") {
    WorkflowHistory h;
    HistoryObserver obs(h);
    WorkflowEvent a; a.type = EventType::WorkAssigned; a.message = "first";
    WorkflowEvent b; b.type = EventType::WorkCompleted; b.message = "second";
    obs.update(a);
    obs.update(b);
    REQUIRE(h.events().size() == 2);
    CHECK(h.events()[0].sequence == 1);
    CHECK(h.events()[1].sequence == 2);
    CHECK(h.eventsOfType(EventType::WorkCompleted).size() == 1);
    CHECK(h.report().find("WORK_ASSIGNED") != std::string::npos);
}

TEST_CASE("Popping a snapshot from an empty history is an error") {
    WorkflowHistory h;
    CHECK(h.snapshotCount() == 0);
    CHECK_THROWS_AS(h.popSnapshot(), WorkflowError);
}
