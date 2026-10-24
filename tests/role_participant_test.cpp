#include "doctest.h"
#include "engine/Participant.h"
#include "engine/Types.h"

using namespace tividy;

TEST_CASE("Role stores typed settings and falls back when missing") {
    Role r("reviewer");
    r.configure("limit", 5000);
    r.configure("label", std::string("senior reviewer"));
    CHECK(r.name() == "reviewer");
    CHECK(r.setting<int>("limit", 0) == 5000);
    CHECK(r.setting<std::string>("label", "") == "senior reviewer");
    CHECK(r.setting<int>("absent", 42) == 42);
    CHECK_FALSE(r.hasSetting("absent"));
}

TEST_CASE("Participant workload never goes negative") {
    Participant p("alice", Role("clerk"));
    p.adjustWorkload(2);
    CHECK(p.workload() == 2);
    p.adjustWorkload(-5);
    CHECK(p.workload() == 0);
    CHECK(p.isAvailable());
}

TEST_CASE("Terminal statuses are recognised") {
    CHECK(isTerminal(WorkStatus::Completed));
    CHECK(isTerminal(WorkStatus::Skipped));
    CHECK(isTerminal(WorkStatus::Cancelled));
    CHECK_FALSE(isTerminal(WorkStatus::Assigned));
}
