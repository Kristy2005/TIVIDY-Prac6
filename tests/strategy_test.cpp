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
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowStage.h"
#include "engine/TIVIDYFacade.h"

TEST_CASE("Assignment strategies can be used polymorphically") {
    tividy::WorkItem item("Review complaint", "Administrator");
    tividy::Participant administrator("Administrator A");
    tividy::Role<void> adminRole("Administrator");
    administrator.addRole(&adminRole);

    std::vector<tividy::Participant*> candidates(1, &administrator);
    tividy::AssignmentStrategy* strategy = new tividy::RoleBasedAssignment();

    CHECK(strategy->assign(&item, candidates) == &administrator);

    delete strategy;
}

TEST_CASE("Role-based assignment selects the first participant with the required role") {
    tividy::Role<void> administratorRole("Administrator");
    tividy::Role<void> staffRole("Staff");
    tividy::Participant first("First staff member");
    tividy::Participant administrator("Administrator");
    tividy::Participant secondAdministrator("Second administrator");
    first.addRole(&staffRole);
    administrator.addRole(&administratorRole);
    secondAdministrator.addRole(&administratorRole);

    tividy::WorkItem review("Review complaint", "Administrator");
    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&first);
    candidates.push_back(nullptr);
    candidates.push_back(&administrator);
    candidates.push_back(&secondAdministrator);

    tividy::RoleBasedAssignment strategy;
    CHECK(strategy.assign(&review, candidates) == &administrator);
}

TEST_CASE("Role-based assignment rejects missing role, null work, and unmatched candidates") {
    tividy::Role<void> staffRole("Staff");
    tividy::Participant staff("Staff member");
    staff.addRole(&staffRole);
    std::vector<tividy::Participant*> candidates(1, &staff);
    tividy::RoleBasedAssignment strategy;
    tividy::WorkItem withoutRole("Unclassified work");
    tividy::WorkItem noMatch("Review", "Department Head");

    CHECK(strategy.assign(nullptr, candidates) == nullptr);
    CHECK(strategy.assign(&withoutRole, candidates) == nullptr);
    CHECK(strategy.assign(&noMatch, candidates) == nullptr);

    std::vector<tividy::Participant*> emptyCandidates;
    tividy::WorkItem review("Review", "Staff");
    CHECK(strategy.assign(&review, emptyCandidates) == nullptr);
    review.complete();
    CHECK(strategy.assign(&review, candidates) == nullptr);
}

TEST_CASE("Role changes on a work item affect role-based routing") {
    tividy::Role<void> administratorRole("Administrator");
    tividy::Role<void> staffRole("Staff");
    tividy::Participant staff("Staff");
    tividy::Participant administrator("Administrator");
    staff.addRole(&staffRole);
    administrator.addRole(&administratorRole);

    tividy::WorkItem item("Review complaint");
    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&staff);
    candidates.push_back(&administrator);
    tividy::RoleBasedAssignment strategy;

    CHECK(strategy.assign(&item, candidates) == nullptr);
    item.setRequiredRole("Administrator");
    CHECK(strategy.assign(&item, candidates) == &administrator);
    item.setRequiredRole("Staff");
    CHECK(strategy.assign(&item, candidates) == &staff);
}

TEST_CASE("Workload-based assignment chooses the smallest outstanding workload") {
    tividy::Participant lightlyLoaded("Light workload");
    tividy::Participant heavilyLoaded("Heavy workload");
    tividy::WorkItem heavyOne("Heavy work 1");
    tividy::WorkItem heavyTwo("Heavy work 2");
    tividy::WorkItem lightOne("Light work");

    heavilyLoaded.assignWork(&heavyOne);
    heavilyLoaded.assignWork(&heavyTwo);
    lightlyLoaded.assignWork(&lightOne);

    tividy::WorkItem newItem("New work");
    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&heavilyLoaded);
    candidates.push_back(nullptr);
    candidates.push_back(&lightlyLoaded);

    tividy::WorkloadBasedAssignment strategy;
    CHECK(heavilyLoaded.getWorkload() == 2);
    CHECK(lightlyLoaded.getWorkload() == 1);
    CHECK(strategy.assign(&newItem, candidates) == &lightlyLoaded);

    // Selecting an assignee does not mutate workload; the caller records it.
    CHECK(lightlyLoaded.getWorkload() == 1);
    lightlyLoaded.assignWork(&newItem);
    CHECK(lightlyLoaded.getWorkload() == 2);
}

TEST_CASE("Workload-based assignment is stable for ties and rejects invalid input") {
    tividy::Participant first("First");
    tividy::Participant second("Second");
    tividy::WorkItem item("New work");
    tividy::WorkloadBasedAssignment strategy;

    std::vector<tividy::Participant*> tiedCandidates;
    tiedCandidates.push_back(nullptr);
    tiedCandidates.push_back(&first);
    tiedCandidates.push_back(&second);
    CHECK(strategy.assign(&item, tiedCandidates) == &first);
    CHECK(strategy.assign(nullptr, tiedCandidates) == nullptr);

    std::vector<tividy::Participant*> emptyCandidates;
    CHECK(strategy.assign(&item, emptyCandidates) == nullptr);
    std::vector<tividy::Participant*> nullCandidates(2, nullptr);
    CHECK(strategy.assign(&item, nullCandidates) == nullptr);

    item.complete();
    CHECK(strategy.assign(&item, tiedCandidates) == nullptr);
}

TEST_CASE("Participant workload counts unique incomplete assignments") {
    tividy::Participant participant("Investigator");
    tividy::WorkItem first("Investigate");
    tividy::WorkItem second("Collect evidence");
    tividy::WorkItem alreadyComplete("Already complete");
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
    tividy::Participant participant("Administrator");
    tividy::Role<void> administratorRole("Administrator");

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
        tividy::Role<void> reviewerRole("Reviewer");
        participant.addRole(&reviewerRole);
    }
    CHECK(participant.hasRole("Reviewer"));
}

TEST_CASE("WorkflowDefinition delegates to the currently configured strategy") {
    tividy::Role<void> adminRole("Administrator");
    tividy::Participant administrator("Administrator");
    tividy::Participant generalStaff("General staff");
    administrator.addRole(&adminRole);

    tividy::WorkItem item("Review complaint", "Administrator");
    tividy::WorkItem staffLoad("Existing staff work");
    tividy::WorkItem adminLoadOne("Existing administrator work 1");
    tividy::WorkItem adminLoadTwo("Existing administrator work 2");
    generalStaff.assignWork(&staffLoad);
    administrator.assignWork(&adminLoadOne);
    administrator.assignWork(&adminLoadTwo);

    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&administrator);
    candidates.push_back(&generalStaff);

    tividy::RoleBasedAssignment roleStrategy;
    tividy::WorkloadBasedAssignment workloadStrategy;
    tividy::WorkflowDefinition definition;

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
    tividy::RoleBasedAssignment strategy;
    tividy::WorkflowBuilder* builder = new tividy::ConcreteWorkflowBuilder();

    builder->setAssignmentStrategy(&strategy);
    tividy::WorkflowDefinition* definition = builder->getResult();
    REQUIRE(definition != nullptr);
    CHECK(definition->getAssignmentStrategy() == &strategy);
    delete definition;
    delete builder;
}

TEST_CASE("Role-based assignment checks completion per workflow instance") {
    tividy::Role<void> adminRole("Administrator");

    tividy::Participant administrator("Administrator");
    administrator.addRole(&adminRole);

    tividy::WorkItem item("Review complaint", "Administrator");

    tividy::WorkflowStage stage;
    stage.add(&item);

    tividy::WorkflowDefinition definition;
    definition.setRoot(&stage);

    tividy::WorkflowInstance first(&definition);
    tividy::WorkflowInstance second(&definition);

    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&administrator);

    tividy::RoleBasedAssignment strategy;

    CHECK(strategy.assign(&first, &item, candidates) == &administrator);
    CHECK(strategy.assign(&second, &item, candidates) == &administrator);

    first.completeWorkItem(&item);

    CHECK(strategy.assign(&first, &item, candidates) == nullptr);
    CHECK(strategy.assign(&second, &item, candidates) == &administrator);
}

TEST_CASE("Workload-based assignment checks completion per workflow instance") {
    tividy::Participant firstParticipant("First");
    tividy::Participant secondParticipant("Second");

    tividy::WorkItem item("Investigate complaint");

    tividy::WorkflowStage stage;
    stage.add(&item);

    tividy::WorkflowDefinition definition;
    definition.setRoot(&stage);

    tividy::WorkflowInstance firstInstance(&definition);
    tividy::WorkflowInstance secondInstance(&definition);

    std::vector<tividy::Participant*> candidates;
    candidates.push_back(&firstParticipant);
    candidates.push_back(&secondParticipant);

    tividy::WorkloadBasedAssignment strategy;

    CHECK(strategy.assign(
        &firstInstance,
        &item,
        candidates
    ) == &firstParticipant);

    CHECK(strategy.assign(
        &secondInstance,
        &item,
        candidates
    ) == &firstParticipant);

    firstInstance.completeWorkItem(&item);

    CHECK(strategy.assign(
        &firstInstance,
        &item,
        candidates
    ) == nullptr);

    CHECK(strategy.assign(
        &secondInstance,
        &item,
        candidates
    ) == &firstParticipant);
}
TEST_CASE("Participant keeps assignments separate between workflow instances") {
    tividy::Participant participant("Investigator");

    tividy::WorkItem item("Investigate");

    tividy::WorkflowStage stage;
    stage.add(&item);

    tividy::WorkflowDefinition definition;
    definition.setRoot(&stage);

    tividy::WorkflowInstance first(&definition);
    tividy::WorkflowInstance second(&definition);

    participant.assignWork(&first, &item);
    participant.assignWork(&second, &item);

    CHECK(participant.getWorkload() == 2);

    participant.completeWork(&first, &item);

    CHECK(participant.getWorkload() == 1);
    CHECK(first.isWorkItemComplete(&item));
    CHECK_FALSE(second.isWorkItemComplete(&item));

    participant.completeWork(&second, &item);

    CHECK(participant.getWorkload() == 0);
    CHECK(second.isWorkItemComplete(&item));
}

TEST_CASE("Dependencies prevent assignment until prerequisite work is complete") {
    tividy::Role<void> staffRole("Staff");

    tividy::Participant staff("Staff member");
    staff.addRole(&staffRole);

    tividy::WorkItem first("First task", "Staff");
    tividy::WorkItem second("Second task", "Staff");

    tividy::ConcreteWorkflowBuilder builder;
    builder.addDependency(&first, &second);

    tividy::RoleBasedAssignment strategy;
    builder.setAssignmentStrategy(&strategy);

    tividy::WorkflowDefinition* definition = builder.getResult();
    REQUIRE(definition != nullptr);

    tividy::WorkflowInstance instance(definition);
    tividy::TIVIDYFacade facade;

    std::vector<tividy::Participant*> participants;
    participants.push_back(&staff);

    // Second depends on first, so it cannot be assigned yet.
    CHECK(
        facade.assignWork(
            &instance,
            &second,
            participants
        ) == nullptr
    );

    // First has no dependency, so it can be assigned.
    tividy::Participant* firstAssignee =
        facade.assignWork(
            &instance,
            &first,
            participants
        );

    REQUIRE(firstAssignee == &staff);

    facade.completeWork(
        &instance,
        &first,
        firstAssignee
    );

    CHECK(instance.isWorkItemComplete(&first));

    // Now that first is complete, second becomes available.
    CHECK(
        facade.assignWork(
            &instance,
            &second,
            participants
        ) == &staff
    );

    delete definition;
}