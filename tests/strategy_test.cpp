#include "doctest.h"

#include <vector>

#include "engine/AssignmentStrategy.h"
#include "engine/ConcreteWorkflowBuilder.h"
#include "engine/Participant.h"
#include "engine/Role.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/WorkItem.h"
#include "engine/WorkloadBasedAssignment.h"
#include "engine/WorkflowBuilder.h"
#include "engine/WorkflowDefinition.h"

TEST_CASE("Assignment strategies can be used polymorphically") {
    WorkItem item("Review complaint", "Administrator");
    Participant administrator("Administrator A");
    Role<void> adminRole("Administrator");
    administrator.addRole(&adminRole);

    std::vector<Participant*> candidates(1, &administrator);
    AssignmentStrategy* strategy = new RoleBasedAssignment();

    CHECK(strategy->assign(&item, candidates) == &administrator);

    delete strategy;
}

TEST_CASE("Role-based assignment selects the first participant with the required role") {
    Role<void> administratorRole("Administrator");
    Role<void> staffRole("Staff");
    Participant first("First staff member");
    Participant administrator("Administrator");
    Participant secondAdministrator("Second administrator");
    first.addRole(&staffRole);
    administrator.addRole(&administratorRole);
    secondAdministrator.addRole(&administratorRole);

    WorkItem review("Review complaint", "Administrator");
    std::vector<Participant*> candidates;
    candidates.push_back(&first);
    candidates.push_back(nullptr);
    candidates.push_back(&administrator);
    candidates.push_back(&secondAdministrator);

    RoleBasedAssignment strategy;
    CHECK(strategy.assign(&review, candidates) == &administrator);
}

TEST_CASE("Role-based assignment rejects missing role, null work, and unmatched candidates") {
    Role<void> staffRole("Staff");
    Participant staff("Staff member");
    staff.addRole(&staffRole);
    std::vector<Participant*> candidates(1, &staff);
    RoleBasedAssignment strategy;
    WorkItem withoutRole("Unclassified work");
    WorkItem noMatch("Review", "Department Head");

    CHECK(strategy.assign(nullptr, candidates) == nullptr);
    CHECK(strategy.assign(&withoutRole, candidates) == nullptr);
    CHECK(strategy.assign(&noMatch, candidates) == nullptr);

    std::vector<Participant*> emptyCandidates;
    WorkItem review("Review", "Staff");
    CHECK(strategy.assign(&review, emptyCandidates) == nullptr);
    review.complete();
    CHECK(strategy.assign(&review, candidates) == nullptr);
}

TEST_CASE("Role changes on a work item affect role-based routing") {
    Role<void> administratorRole("Administrator");
    Role<void> staffRole("Staff");
    Participant staff("Staff");
    Participant administrator("Administrator");
    staff.addRole(&staffRole);
    administrator.addRole(&administratorRole);

    WorkItem item("Review complaint");
    std::vector<Participant*> candidates;
    candidates.push_back(&staff);
    candidates.push_back(&administrator);
    RoleBasedAssignment strategy;

    CHECK(strategy.assign(&item, candidates) == nullptr);
    item.setRequiredRole("Administrator");
    CHECK(strategy.assign(&item, candidates) == &administrator);
    item.setRequiredRole("Staff");
    CHECK(strategy.assign(&item, candidates) == &staff);
}

TEST_CASE("Workload-based assignment chooses the smallest outstanding workload") {
    Participant lightlyLoaded("Light workload");
    Participant heavilyLoaded("Heavy workload");
    WorkItem heavyOne("Heavy work 1");
    WorkItem heavyTwo("Heavy work 2");
    WorkItem lightOne("Light work");

    heavilyLoaded.assignWork(&heavyOne);
    heavilyLoaded.assignWork(&heavyTwo);
    lightlyLoaded.assignWork(&lightOne);

    WorkItem newItem("New work");
    std::vector<Participant*> candidates;
    candidates.push_back(&heavilyLoaded);
    candidates.push_back(nullptr);
    candidates.push_back(&lightlyLoaded);

    WorkloadBasedAssignment strategy;
    CHECK(heavilyLoaded.getWorkload() == 2);
    CHECK(lightlyLoaded.getWorkload() == 1);
    CHECK(strategy.assign(&newItem, candidates) == &lightlyLoaded);

    // Selecting an assignee does not mutate workload; the caller records it.
    CHECK(lightlyLoaded.getWorkload() == 1);
    lightlyLoaded.assignWork(&newItem);
    CHECK(lightlyLoaded.getWorkload() == 2);
}

TEST_CASE("Workload-based assignment is stable for ties and rejects invalid input") {
    Participant first("First");
    Participant second("Second");
    WorkItem item("New work");
    WorkloadBasedAssignment strategy;

    std::vector<Participant*> tiedCandidates;
    tiedCandidates.push_back(nullptr);
    tiedCandidates.push_back(&first);
    tiedCandidates.push_back(&second);
    CHECK(strategy.assign(&item, tiedCandidates) == &first);
    CHECK(strategy.assign(nullptr, tiedCandidates) == nullptr);

    std::vector<Participant*> emptyCandidates;
    CHECK(strategy.assign(&item, emptyCandidates) == nullptr);
    std::vector<Participant*> nullCandidates(2, nullptr);
    CHECK(strategy.assign(&item, nullCandidates) == nullptr);

    item.complete();
    CHECK(strategy.assign(&item, tiedCandidates) == nullptr);
}

TEST_CASE("Participant workload counts unique incomplete assignments") {
    Participant participant("Investigator");
    WorkItem first("Investigate");
    WorkItem second("Collect evidence");
    WorkItem alreadyComplete("Already complete");
    alreadyComplete.complete();

    CHECK(participant.getWorkload() == 0);
    participant.assignWork(nullptr);
    participant.assignWork(&alreadyComplete);
    CHECK(participant.getWorkload() == 0);

    participant.assignWork(&first);
    participant.assignWork(&first);
    participant.assignWork(&second);
    CHECK(participant.getWorkload() == 2);

    participant.completeWork(nullptr);
    participant.completeWork(&alreadyComplete);
    CHECK(participant.getWorkload() == 2);
    CHECK_FALSE(first.isComplete());

    participant.completeWork(&first);
    CHECK(first.isComplete());
    CHECK(participant.getWorkload() == 1);
    participant.completeWork(&first);
    CHECK(participant.getWorkload() == 1);

    participant.completeWork(&second);
    CHECK(second.isComplete());
    CHECK(participant.getWorkload() == 0);
}

TEST_CASE("Participant records roles without duplicates or Role pointer ownership") {
    Participant participant("Administrator");
    Role<void> administratorRole("Administrator");

    CHECK(participant.getName() == "Administrator");
    CHECK_FALSE(participant.hasRole("Administrator"));
    participant.addRole(&administratorRole);
    participant.addRole(&administratorRole);
    participant.addRole(std::string());

    CHECK(participant.hasRole("Administrator"));
    CHECK_FALSE(participant.hasRole("Staff"));
    CHECK_FALSE(participant.hasRole(std::string()));

    // The role name is copied rather than retaining a Role pointer.
    {
        Role<void> reviewerRole("Reviewer");
        participant.addRole(&reviewerRole);
    }
    CHECK(participant.hasRole("Reviewer"));
}

TEST_CASE("WorkflowDefinition delegates to the currently configured strategy") {
    Role<void> adminRole("Administrator");
    Participant administrator("Administrator");
    Participant generalStaff("General staff");
    administrator.addRole(&adminRole);

    WorkItem item("Review complaint", "Administrator");
    WorkItem staffLoad("Existing staff work");
    WorkItem adminLoadOne("Existing administrator work 1");
    WorkItem adminLoadTwo("Existing administrator work 2");
    generalStaff.assignWork(&staffLoad);
    administrator.assignWork(&adminLoadOne);
    administrator.assignWork(&adminLoadTwo);

    std::vector<Participant*> candidates;
    candidates.push_back(&administrator);
    candidates.push_back(&generalStaff);

    RoleBasedAssignment roleStrategy;
    WorkloadBasedAssignment workloadStrategy;
    WorkflowDefinition definition;

    CHECK(definition.getAssignmentStrategy() == nullptr);
    CHECK(definition.selectAssignee(&item, candidates) == nullptr);

    definition.setAssignmentStrategy(&roleStrategy);
    CHECK(definition.getAssignmentStrategy() == &roleStrategy);
    CHECK(definition.selectAssignee(&item, candidates) == &administrator);

    definition.setAssignmentStrategy(&workloadStrategy);
    CHECK(definition.getAssignmentStrategy() == &workloadStrategy);
    CHECK(definition.selectAssignee(&item, candidates) == &generalStaff);

    definition.setAssignmentStrategy(nullptr);
    CHECK(definition.selectAssignee(&item, candidates) == nullptr);
}

TEST_CASE("ConcreteWorkflowBuilder configures the definition through the builder interface") {
    RoleBasedAssignment strategy;
    WorkflowBuilder* builder = new ConcreteWorkflowBuilder();

    builder->setAssignmentStrategy(&strategy);
    WorkflowDefinition* definition = builder->getResult();
    REQUIRE(definition != nullptr);
    CHECK(definition->getAssignmentStrategy() == &strategy);
    delete definition;
    delete builder;
}
