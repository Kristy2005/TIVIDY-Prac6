#include "doctest.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "engine/ActiveState.h"
#include "engine/InProgressState.h"
#include "engine/ResolvedState.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/WorkItem.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowStage.h"
#include "engine/TIVIDYFacade.h"

using namespace tividy;

TEST_CASE("Runtime additions belong to one instance and do not alter its definition") {
    TIVIDYFacade facade;
    RoleBasedAssignment strategy;
    WorkflowStage root;
    WorkItem registration("Register", "Clerk");
    WorkItem verification("Verify", "Reviewer");
    verification.addDependency(&registration);
    root.add(&registration);
    root.add(&verification);

    WorkflowDefinition definition("Application workflow");
    definition.setRoot(&root);
    definition.setAssignmentStrategy(&strategy);

    std::unique_ptr<WorkflowInstance> first(facade.startWorkflow(&definition));
    std::unique_ptr<WorkflowInstance> second(facade.startWorkflow(&definition));
    first->startProcessing();
    second->startProcessing();

    WorkItem* added = facade.addWorkItemToInstance(
        first.get(), "Schedule interview", "Coordinator", &verification);
    REQUIRE(added != nullptr);
    CHECK(first->getInstanceWorkItem(added) == added);
    CHECK(first->getWorkItemProgress().size() == 3);
    CHECK(second->getWorkItemProgress().size() == 2);
    CHECK(facade.getAllWorkItems(first.get()).size() == 3);
    CHECK(facade.getAllWorkItems(second.get()).size() == 2);
    CHECK(added->getRequiredRole() == "Coordinator");
    CHECK(added->getPriority() == 0);
    CHECK_THROWS_AS(
        facade.addWorkItemToInstance(first.get(), "Schedule interview", "Coordinator"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        facade.addWorkItemToInstance(first.get(), "   ", "Coordinator"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        facade.addWorkItemToInstance(first.get(), "Valid name", "  "),
        std::invalid_argument
    );

    // Completing the prerequisite in the first case makes its new item ready;
    // the second case still has independent progress.
    facade.completeWork(first.get(), &registration);
    CHECK(first->areDependenciesComplete(added) == false);
    facade.completeWork(first.get(), &verification);
    CHECK(first->areDependenciesComplete(added));
    CHECK_FALSE(second->isWorkItemComplete(&registration));

    // A fresh instance is also based on the unchanged definition.
    std::unique_ptr<WorkflowInstance> third(facade.startWorkflow(&definition));
    CHECK(third->getWorkItemProgress().size() == 2);
    CHECK(facade.getAllWorkItems(third.get()).size() == 2);

    const std::string history = facade.getHistory(first.get());
    CHECK(history.find("Work item added") != std::string::npos);
    CHECK(history.find("Work completed") != std::string::npos);
    CHECK(facade.getHistory(second.get()).find("Schedule interview") == std::string::npos);
}

TEST_CASE("Runtime cancellation rejects completed work and live dependents") {
    TIVIDYFacade facade;
    WorkflowStage root;
    WorkItem prerequisite("Prepare file", "Clerk");
    WorkItem dependent("Review file", "Reviewer");
    dependent.addDependency(&prerequisite);
    root.add(&prerequisite);
    root.add(&dependent);
    WorkflowDefinition definition;
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    CHECK_THROWS_AS(facade.cancelWorkItem(instance.get(), &prerequisite), std::logic_error);
    CHECK_FALSE(instance->isWorkItemCancelled(&prerequisite));

    facade.cancelWorkItem(instance.get(), &dependent);
    CHECK(instance->isWorkItemCancelled(&dependent));
    CHECK(facade.getIncompleteWorkItems(instance.get()).size() == 1);
    CHECK(facade.getAllWorkItems(instance.get()).size() == 1);
    CHECK(facade.getHistory(instance.get()).find("Work cancelled") != std::string::npos);

    facade.completeWork(instance.get(), &prerequisite);
    CHECK_THROWS_AS(facade.cancelWorkItem(instance.get(), &prerequisite), std::logic_error);
    CHECK_THROWS_AS(facade.completeWork(instance.get(), &dependent), std::logic_error);
}

TEST_CASE("Runtime reassignment updates workload and role changes release invalid assignees") {
    TIVIDYFacade facade;
    WorkflowStage root;
    WorkItem review("Review application", "Reviewer");
    root.add(&review);
    WorkflowDefinition definition;
    definition.setRoot(&root);

    Participant alice("Alice");
    alice.addRole("Reviewer");
    Participant bob("Bob");
    bob.addRole("Reviewer");
    bob.addRole("Manager");

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();
    facade.addParticipantToInstance(instance.get(), &alice);
    facade.addParticipantToInstance(instance.get(), &bob);

    WorkItem* item = facade.getAllWorkItems(instance.get())[0];
    facade.reassignWorkItem(instance.get(), item, &alice);
    CHECK(facade.getAssignee(instance.get(), item) == &alice);
    CHECK(alice.getWorkload() == 1);
    CHECK(bob.getWorkload() == 0);

    facade.reassignWorkItem(instance.get(), item, &bob);
    CHECK(facade.getAssignee(instance.get(), item) == &bob);
    CHECK(alice.getWorkload() == 0);
    CHECK(bob.getWorkload() == 1);

    facade.changeWorkItemRole(instance.get(), item, "Manager");
    CHECK(facade.getAssignee(instance.get(), item) == &bob);
    CHECK(bob.getWorkload() == 1);

    facade.changeWorkItemRole(instance.get(), item, "Clerk");
    CHECK(facade.getAssignee(instance.get(), item) == nullptr);
    CHECK(bob.getWorkload() == 0);

    facade.changeWorkItemPriority(instance.get(), item, 7);
    CHECK(item->getPriority() == 7);
    CHECK_THROWS_AS(facade.changeWorkItemPriority(instance.get(), item, -1), std::invalid_argument);
    CHECK_THROWS_AS(facade.reassignWorkItem(instance.get(), item, &alice), std::invalid_argument);

    // Cancelling an assigned item also releases its participant workload.
    facade.changeWorkItemRole(instance.get(), item, "Manager");
    facade.reassignWorkItem(instance.get(), item, &bob);
    CHECK(bob.getWorkload() == 1);
    facade.cancelWorkItem(instance.get(), item);
    CHECK(facade.getAssignee(instance.get(), item) == nullptr);
    CHECK(bob.getWorkload() == 0);

    const std::string history = facade.getHistory(instance.get());
    CHECK(history.find("Work assigned") != std::string::npos);
    CHECK(history.find("Work reassigned") != std::string::npos);
    CHECK(history.find("Work role changed to Clerk") != std::string::npos);
    CHECK(history.find("Work priority changed to 7") != std::string::npos);
    CHECK(history.find("Work cancelled") != std::string::npos);
}

TEST_CASE("Role, priority and assignment changes stay inside the selected instance") {
    TIVIDYFacade facade;
    WorkflowStage root;
    WorkItem task("Review request", "Reviewer");
    root.add(&task);
    WorkflowDefinition definition;
    definition.setRoot(&root);

    // Participants outlive the instances because the test registers them as
    // non-owning pointers, matching the public engine API contract.
    Participant firstCaseStaff("Case staff");
    firstCaseStaff.addRole("Reviewer");
    firstCaseStaff.addRole("Coordinator");
    Participant secondCaseStaff("Case staff");
    secondCaseStaff.addRole("Reviewer");

    std::unique_ptr<WorkflowInstance> first(facade.startWorkflow(&definition));
    std::unique_ptr<WorkflowInstance> second(facade.startWorkflow(&definition));
    first->startProcessing();
    second->startProcessing();
    facade.addParticipantToInstance(first.get(), &firstCaseStaff);
    facade.addParticipantToInstance(second.get(), &secondCaseStaff);

    WorkItem* firstTask = facade.getAllWorkItems(first.get())[0];
    WorkItem* secondTask = facade.getAllWorkItems(second.get())[0];
    facade.changeWorkItemRole(first.get(), firstTask, "Coordinator");
    facade.changeWorkItemPriority(first.get(), firstTask, 9);
    facade.reassignWorkItem(first.get(), firstTask, &firstCaseStaff);

    CHECK(firstTask->getRequiredRole() == "Coordinator");
    CHECK(firstTask->getPriority() == 9);
    CHECK(facade.getAssignee(first.get(), firstTask) == &firstCaseStaff);
    CHECK(secondTask->getRequiredRole() == "Reviewer");
    CHECK(secondTask->getPriority() == 0);
    CHECK(facade.getAssignee(second.get(), secondTask) == nullptr);
    CHECK(firstCaseStaff.getWorkload() == 1);
    CHECK(secondCaseStaff.getWorkload() == 0);
    CHECK(facade.getHistory(second.get()).find("Work priority changed") == std::string::npos);
}

TEST_CASE("A dynamically added participant is owned by only its selected instance") {
    TIVIDYFacade facade;
    WorkflowStage root;
    WorkItem task("Initial task", "Staff");
    root.add(&task);
    WorkflowDefinition definition;
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> first(facade.startWorkflow(&definition));
    std::unique_ptr<WorkflowInstance> second(facade.startWorkflow(&definition));
    first->startProcessing();
    second->startProcessing();

    Participant* temporary = new Participant("Temporary staff member");
    temporary->addRole("Staff");
    facade.addParticipantToInstance(first.get(), temporary, true);

    CHECK(facade.getParticipantObjects(first.get()).size() == 1);
    CHECK(facade.getParticipantObjects(second.get()).empty());
    CHECK(facade.getHistory(first.get()).find("Participant added") != std::string::npos);
    CHECK(facade.getHistory(second.get()).find("Temporary staff member") == std::string::npos);
}

TEST_CASE("Only active instances can be edited and completion resolves after the last active item") {
    TIVIDYFacade facade;
    WorkflowStage root;
    WorkItem task("Only task", "Staff");
    root.add(&task);
    WorkflowDefinition definition;
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();
    WorkItem* actualTask = facade.getAllWorkItems(instance.get())[0];
    facade.completeWork(instance.get(), actualTask);
    CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
    CHECK_THROWS_AS(
        facade.addWorkItemToInstance(instance.get(), "Late task", "Staff"),
        std::logic_error
    );
}
