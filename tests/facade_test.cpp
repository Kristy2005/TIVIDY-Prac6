#include "doctest.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>

#include "engine/ActiveState.h"
#include "engine/ClosedState.h"
#include "engine/InProgressState.h"
#include "engine/ResolvedState.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/AssignmentStrategy.h"
#include "engine/ApprovalHandler.h"
#include "engine/Handler.h"
#include "engine/Role.h"
#include "engine/WorkItem.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowDirector.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowStage.h"
#include "engine/TIVIDYFacade.h"

using namespace tividy;
typedef tividy::WorkItem WorkItemType;

// ---------------------------------------------------------------------------
// cancelWorkItem's resolve guard (the bug you just fixed: cancelling the last
// outstanding item while the instance is still Active used to crash resolve()).
// ---------------------------------------------------------------------------

TEST_CASE("cancelling the only work item leaves the instance unresolved (no active work remains)") {
    // areAllWorkItemsComplete() requires at least one non-cancelled item, so
    // cancelling the sole item never triggers resolve(). This documents that
    // intentional behaviour rather than asserting the opposite.
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType only("Only item");
    root.add(&only);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    CHECK_NOTHROW(facade.cancelWorkItem(instance.get(), &only));
    CHECK(dynamic_cast<ActiveState*>(instance->getState()) != nullptr);
    CHECK(instance->isWorkItemCancelled(&only));
}

TEST_CASE("cancelWorkItem resolves an Active instance when the last outstanding item is cancelled "
          "alongside an already-completed item") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType first("Keep");
    WorkItemType second("Drop");
    root.add(&first);
    root.add(&second);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    // Deliberately NOT calling startProcessing(): instance stays Active,
    // which used to make resolve() throw once the last item was cancelled.
    CHECK(dynamic_cast<ActiveState*>(instance->getState()) != nullptr);

    facade.completeWork(instance.get(), &first);
    CHECK(dynamic_cast<ActiveState*>(instance->getState()) != nullptr); // not resolved yet

    CHECK_NOTHROW(facade.cancelWorkItem(instance.get(), &second));
    CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
}

TEST_CASE("cancelWorkItem resolves an InProgress instance the same way") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType first("Keep");
    WorkItemType second("Drop");
    root.add(&first);
    root.add(&second);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();
    CHECK(dynamic_cast<InProgressState*>(instance->getState()) != nullptr);

    facade.completeWork(instance.get(), &first);
    CHECK_NOTHROW(facade.cancelWorkItem(instance.get(), &second));
    CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
}

TEST_CASE("cancelWorkItem does not resolve while other items remain incomplete") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType first("First");
    WorkItemType second("Second");
    root.add(&first);
    root.add(&second);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    facade.cancelWorkItem(instance.get(), &first);
    CHECK(dynamic_cast<InProgressState*>(instance->getState()) != nullptr);
    CHECK(instance->isWorkItemCancelled(&first));
    CHECK_FALSE(instance->isWorkItemComplete(&second));
}

// ---------------------------------------------------------------------------
// resolveIfComplete consistency: all three call sites (2-arg completeWork,
// 3-arg completeWork, cancelWorkItem) should behave identically when they
// finish the last item on an Active instance.
// ---------------------------------------------------------------------------

TEST_CASE("resolveIfComplete behaves the same from completeWork and cancelWorkItem") {
    RoleBasedAssignment strategy;
    Participant worker("Worker");
    worker.addRole("Worker");

    SUBCASE("via two-arg completeWork") {
        TIVIDYFacade facade;
        WorkflowDefinition definition;
        WorkflowStage root;
        WorkItemType item("Task");
        root.add(&item);
        definition.setRoot(&root);
        std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
        CHECK_NOTHROW(facade.completeWork(instance.get(), &item));
        CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
    }

    SUBCASE("via three-arg completeWork") {
        TIVIDYFacade facade;
        WorkflowDefinition definition;
        WorkflowStage root;
        WorkItemType item("Task", "Worker");
        root.add(&item);
        definition.setRoot(&root);
        definition.setAssignmentStrategy(&strategy);
        std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));

        std::vector<Participant*> candidates(1, &worker);
        REQUIRE(facade.assignWork(instance.get(), &item, candidates) == &worker);
        CHECK_NOTHROW(facade.completeWork(instance.get(), &item, &worker));
        CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
    }

    SUBCASE("via cancelWorkItem") {
        // A lone cancelled item never resolves (no active work remains), so
        // this needs a completed item alongside the one being cancelled,
        // same as the dedicated cancelWorkItem tests above.
        TIVIDYFacade facade;
        WorkflowDefinition definition;
        WorkflowStage root;
        WorkItemType keep("Keep");
        WorkItemType drop("Drop");
        root.add(&keep);
        root.add(&drop);
        definition.setRoot(&root);
        std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
        facade.completeWork(instance.get(), &keep);
        CHECK_NOTHROW(facade.cancelWorkItem(instance.get(), &drop));
        CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
    }
}

// ---------------------------------------------------------------------------
// Definition-building passthrough to the builder (builderFor / configuring mode)
// ---------------------------------------------------------------------------

TEST_CASE("Facade builds a definition through the builder-backed methods") {
    TIVIDYFacade facade;
    // The unique_ptr owns the definition and deletes it at end of scope.
    std::unique_ptr<WorkflowDefinition> owned(facade.createWorkflow("Onboarding"));
    WorkflowDefinition* definition = owned.get();
    REQUIRE(definition != nullptr);
    CHECK(facade.getDefinitionName(definition) == "Onboarding");

    WorkItem* step1 = facade.createWorkItemInDefinition(definition, "Collect ID", "Clerk");
    WorkItem* step2 = facade.createWorkItemInDefinition(definition, "Verify ID", "Clerk");
    REQUIRE(step1 != nullptr);
    REQUIRE(step2 != nullptr);

    CHECK_NOTHROW(facade.addDependencyToWorkItem(definition, step2, step1));
    CHECK_NOTHROW(facade.addPriorityToWorkItem(definition, step1, 5));

    RoleBasedAssignment strategy;
    CHECK_NOTHROW(facade.configureAssignmentStrategy(definition, &strategy));

    ApprovalHandler handler;
    CHECK_NOTHROW(facade.configureEscalationHandler(definition, &handler));

    Participant* clerk = facade.createParticipantInDefinition(definition, "Clerk One", "Clerk");
    REQUIRE(clerk != nullptr);

    std::vector<Participant*> participants = facade.getParticipantsForDefinition(definition);
    CHECK(participants.size() == 1);
    CHECK(participants[0] == clerk);

    std::vector<Participant*> everyone = facade.getAllParticipants();
    CHECK(std::find(everyone.begin(), everyone.end(), clerk) != everyone.end());
}

TEST_CASE("Facade sub-process nesting through beginSubProcessInDefinition/endSubProcessInDefinition") {
    TIVIDYFacade facade;
    // The unique_ptr owns the definition and deletes it at end of scope.
    std::unique_ptr<WorkflowDefinition> owned(facade.createWorkflow("Review"));
    WorkflowDefinition* definition = owned.get();

    WorkItem* outer = facade.createWorkItemInDefinition(definition, "Outer step");
    REQUIRE(outer != nullptr);

    CHECK_NOTHROW(facade.beginSubProcessInDefinition(definition));
    WorkItem* nested = facade.createWorkItemInDefinition(definition, "Nested step");
    REQUIRE(nested != nullptr);
    CHECK_NOTHROW(facade.endSubProcessInDefinition(definition));

    // Ending with no open sub-process should fail rather than silently no-op.
    CHECK_THROWS(facade.endSubProcessInDefinition(definition));
}

TEST_CASE("Facade creates a sequential workflow from WorkflowStep recipes") {
    TIVIDYFacade facade;
    std::vector<WorkflowStep> steps;
    steps.push_back(WorkflowStep("Draft", "Author", 1));
    steps.push_back(WorkflowStep("Review", "Reviewer", 2));

    // The unique_ptr owns the definition. It is declared before the instance,
    // so the instance is destroyed first.
    std::unique_ptr<WorkflowDefinition> owned(facade.createSequentialWorkflow("Publishing", steps));
    WorkflowDefinition* definition = owned.get();
    REQUIRE(definition != nullptr);
    CHECK(facade.getDefinitionName(definition) == "Publishing");

    std::vector<WorkflowDefinition*> all = facade.getAllDefinitions();
    CHECK(std::find(all.begin(), all.end(), definition) != all.end());

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(definition));
    std::vector<WorkItem*> items = facade.getAllWorkItems(instance.get());
    CHECK(items.size() == 2);
}

// ---------------------------------------------------------------------------
// Instance-level participant and work-item management
// ---------------------------------------------------------------------------

TEST_CASE("Facade adds, reassigns and reroles work at the instance level") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;

    // Declared before the instance, so it outlives the instance. The instance
    // clears its assignments from external participants in its destructor.
    Participant reviewer("Reviewer One");
    reviewer.addRole("Reviewer");

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    WorkItem* item = facade.addWorkItemToInstance(instance.get(), "Dynamic task", "Reviewer");
    REQUIRE(item != nullptr);

    CHECK_NOTHROW(facade.addParticipantToInstance(instance.get(), &reviewer, false));

    CHECK_NOTHROW(facade.reassignWorkItem(instance.get(), item, &reviewer));
    CHECK(facade.getAssignee(instance.get(), item) == &reviewer);

    CHECK_NOTHROW(facade.changeWorkItemPriority(instance.get(), item, 3));

    // Role change to a role the current assignee still holds: assignment survives.
    CHECK_NOTHROW(facade.changeWorkItemRole(instance.get(), item, "Reviewer"));
    CHECK(facade.getAssignee(instance.get(), item) == &reviewer);

    std::vector<WorkItem*> assigned = facade.getAssignedWorkItems(instance.get());
    CHECK(std::find(assigned.begin(), assigned.end(), item) != assigned.end());

    std::vector<Participant*> objects = facade.getParticipantObjects(instance.get());
    CHECK(std::find(objects.begin(), objects.end(), &reviewer) != objects.end());
}

TEST_CASE("changing a work item's required role auto-releases an incompatible assignee") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;

    // Declared before the instance, so it outlives the instance.
    Participant reviewer("Reviewer One");
    reviewer.addRole("Reviewer");

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    WorkItem* item = facade.addWorkItemToInstance(instance.get(), "Dynamic task", "Reviewer");
    facade.addParticipantToInstance(instance.get(), &reviewer, false);
    facade.reassignWorkItem(instance.get(), item, &reviewer);
    REQUIRE(facade.getAssignee(instance.get(), item) == &reviewer);

    facade.changeWorkItemRole(instance.get(), item, "SeniorReviewer");
    CHECK(facade.getAssignee(instance.get(), item) == nullptr);
}

// ---------------------------------------------------------------------------
// Null-argument validation for the API surface the original test case
// ("Facade validates null arguments...") doesn't cover.
// ---------------------------------------------------------------------------

TEST_CASE("Facade validates null arguments on the newer instance- and definition-level methods") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType item("Task");
    root.add(&item);
    definition.setRoot(&root);

    // Declared before the instance, so it outlives the instance.
    Participant participant("Someone");

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));

    CHECK_THROWS_AS(facade.cancelWorkItem(nullptr, &item), std::invalid_argument);
    CHECK_THROWS_AS(facade.cancelWorkItem(instance.get(), nullptr), std::invalid_argument);

    CHECK_THROWS_AS(facade.reassignWorkItem(nullptr, &item, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.reassignWorkItem(instance.get(), nullptr, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.reassignWorkItem(instance.get(), &item, nullptr), std::invalid_argument);

    CHECK_THROWS_AS(facade.changeWorkItemRole(nullptr, &item, "Role"), std::invalid_argument);
    CHECK_THROWS_AS(facade.changeWorkItemRole(instance.get(), nullptr, "Role"), std::invalid_argument);

    CHECK_THROWS_AS(facade.changeWorkItemPriority(nullptr, &item, 1), std::invalid_argument);
    CHECK_THROWS_AS(facade.changeWorkItemPriority(instance.get(), nullptr, 1), std::invalid_argument);

    CHECK_THROWS_AS(facade.addParticipantToInstance(nullptr, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.addParticipantToInstance(instance.get(), nullptr), std::invalid_argument);

    CHECK_THROWS_AS(facade.getAssignee(nullptr, &item), std::invalid_argument);
    CHECK_THROWS_AS(facade.getAssignee(instance.get(), nullptr), std::invalid_argument);

    CHECK_THROWS_AS(facade.getParticipantObjects(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getAssignedWorkItems(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getHistory(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getProgress(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getIncompleteWorkItems(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getAllWorkItems(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getParticipants(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.addWorkItemToInstance(nullptr, "Task", "Role"), std::invalid_argument);

    CHECK_THROWS_AS(facade.getInstancesForDefinition(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getRolesForDefinition(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getParticipantsForDefinition(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.getDefinitionName(nullptr), std::invalid_argument);

    RoleBasedAssignment strategy;
    ApprovalHandler handler;
    Role<void> role("Reviewer");
    CHECK_THROWS_AS(facade.configureAssignmentStrategy(nullptr, &strategy), std::invalid_argument);
    CHECK_THROWS_AS(facade.configureAssignmentStrategy(&definition, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.configureEscalationHandler(nullptr, &handler), std::invalid_argument);
    CHECK_THROWS_AS(facade.configureEscalationHandler(&definition, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.addRoleToDefinition(nullptr, &role), std::invalid_argument);
    CHECK_THROWS_AS(facade.addRoleToDefinition(&definition, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.addWorkItemToDefinition(nullptr, &item), std::invalid_argument);
    CHECK_THROWS_AS(facade.addWorkItemToDefinition(&definition, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.addPriorityToWorkItem(nullptr, &item, 1), std::invalid_argument);
    CHECK_THROWS_AS(facade.createWorkItemInDefinition(nullptr, "Task"), std::invalid_argument);
    CHECK_THROWS_AS(facade.beginSubProcessInDefinition(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.endSubProcessInDefinition(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.createParticipantInDefinition(nullptr, "Name"), std::invalid_argument);
    CHECK_THROWS_AS(facade.addParticipantToDefinition(nullptr, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.addParticipantToDefinition(&definition, nullptr), std::invalid_argument);
}