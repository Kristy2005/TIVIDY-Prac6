#include "doctest.h"
#include "engine/AssignmentStrategy.h"
#include "engine/WorkItem.h"

using namespace tividy;

namespace {
struct Team {
    Participant alice, bob, carol;
    std::vector<Participant*> all;
    Team() : alice("alice", Role("clerk")), bob("bob", Role("clerk")), carol("carol", Role("manager")) {
        all.push_back(&alice); all.push_back(&bob); all.push_back(&carol);
    }
};
}

TEST_CASE("RoleBasedAssignment picks the first participant with the required role") {
    Team t;
    WorkItem item("i", "Item", "clerk");
    CHECK(RoleBasedAssignment().select(item, t.all) == &t.alice);
    WorkItem boss("b", "Boss item", "manager");
    CHECK(RoleBasedAssignment().select(boss, t.all) == &t.carol);
}

TEST_CASE("WorkloadBasedAssignment prefers the least loaded participant") {
    Team t;
    t.alice.adjustWorkload(3);
    WorkItem item("i", "Item", "clerk");
    CHECK(WorkloadBasedAssignment().select(item, t.all) == &t.bob);
}

TEST_CASE("Strategies skip unavailable participants and return NULL when nobody fits") {
    Team t;
    t.alice.setAvailable(false);
    t.bob.setAvailable(false);
    WorkItem item("i", "Item", "clerk");
    CHECK(RoleBasedAssignment().select(item, t.all) == static_cast<Participant*>(0));
    WorkItem none("n", "Nobody has this role", "auditor");
    CHECK(WorkloadBasedAssignment().select(none, t.all) == static_cast<Participant*>(0));
}
