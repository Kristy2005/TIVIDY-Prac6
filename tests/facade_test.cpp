#include "doctest.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "engine/ActiveState.h"
#include "engine/ApprovalHandler.h"
#include "engine/EscalationHandler.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/ClosedState.h"
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
typedef tividy::WorkItem WorkItemType;

TEST_CASE("TIVIDYFacade creates and starts a workflow in the Active state") {
    TIVIDYFacade facade;
    std::unique_ptr<WorkflowDefinition> definition(facade.createWorkflow());
    REQUIRE(definition.get() != nullptr);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(definition.get()));
    REQUIRE(instance.get() != nullptr);
    CHECK(instance->getDefinition() == definition.get());
    CHECK(dynamic_cast<ActiveState*>(instance->getState()) != nullptr);
}

TEST_CASE("Facade participant assignment and completion update workload and instance progress") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType first("Check documents", "Administrator");
    WorkItemType second("Approve application", "Administrator");
    Participant administrator("Administrator");
    RoleBasedAssignment strategy;

    root.add(&first);
    root.add(&second);
    definition.setRoot(&root);
    definition.setAssignmentStrategy(&strategy);
    administrator.addRole("Administrator");

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    std::vector<Participant*> candidates(1, &administrator);
    CHECK(facade.assignWork(instance.get(), &first, candidates) == &administrator);
    CHECK(facade.assignWork(instance.get(), &second, candidates) == &administrator);
    CHECK(administrator.getWorkload() == 2);
    CHECK_FALSE(instance->isWorkItemComplete(&first));
    CHECK_FALSE(instance->isWorkItemComplete(&second));

    facade.completeWork(instance.get(), &first, &administrator);
    CHECK(administrator.getWorkload() == 1);
    CHECK(instance->isWorkItemComplete(&first));
    CHECK_FALSE(instance->isWorkItemComplete(&second));
    CHECK(dynamic_cast<InProgressState*>(instance->getState()) != nullptr);

    facade.completeWork(instance.get(), &second, &administrator);
    CHECK(administrator.getWorkload() == 0);
    CHECK(instance->isWorkItemComplete(&second));
    CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);

    // Instance progress is authoritative; executing/finishing these tasks
    // through the facade does not mark the shared WorkItem templates complete.
    CHECK_FALSE(first.isComplete());
    CHECK_FALSE(second.isComplete());
}

TEST_CASE("Facade simple assignment executes a work item without completing it") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType item("Collect supporting documents");
    root.add(&item);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();

    CHECK_NOTHROW(facade.assignWork(instance.get(), &item));
    CHECK_FALSE(item.isComplete());
    CHECK_FALSE(instance->isWorkItemComplete(&item));

    facade.completeWork(instance.get(), &item);
    CHECK(instance->isWorkItemComplete(&item));
    CHECK(dynamic_cast<ResolvedState*>(instance->getState()) != nullptr);
}

TEST_CASE("Facade returns null when no assignment strategy or suitable participant exists") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    WorkflowStage root;
    WorkItemType item("Review request", "Administrator");
    root.add(&item);
    definition.setRoot(&root);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    Participant participant("Unassigned staff member");
    std::vector<Participant*> candidates(1, &participant);

    CHECK(facade.assignWork(instance.get(), &item, candidates) == nullptr);

    RoleBasedAssignment strategy;
    definition.setAssignmentStrategy(&strategy);
    CHECK(facade.assignWork(instance.get(), &item, candidates) == nullptr);
}

TEST_CASE("Facade escalation delegates to the configured handler chain") {
    class CountingFinalHandler : public FinalDecisionHandler {
    public:
        CountingFinalHandler() : calls(0) {}
        virtual void handleRequest(WorkflowInstance* instance) override {
            ++calls;
            FinalDecisionHandler::handleRequest(instance);
        }
        int calls;
    };

    TIVIDYFacade facade;
    WorkflowDefinition definition;
    ApprovalHandler approval;
    EscalationHandler escalation;
    CountingFinalHandler finalDecision;
    approval.setSuccessor(&escalation);
    escalation.setSuccessor(&finalDecision);
    definition.setEscalationHandler(&approval);

    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));
    instance->startProcessing();
    facade.escalateWork(instance.get());

    CHECK(finalDecision.calls == 1);
    CHECK(dynamic_cast<InProgressState*>(instance->getState()) != nullptr);
}

TEST_CASE("Facade closes an active workflow through the public interface") {
    TIVIDYFacade facade;
    WorkflowDefinition definition;
    std::unique_ptr<WorkflowInstance> instance(facade.startWorkflow(&definition));

    facade.closeWorkflow(instance.get());
    CHECK(dynamic_cast<ClosedState*>(instance->getState()) != nullptr);
}

TEST_CASE("Facade validates null arguments and missing workflow definitions") {
    TIVIDYFacade facade;
    WorkflowInstance instance;
    WorkItemType item("Review request");
    Participant participant("Reviewer");
    std::vector<Participant*> candidates(1, &participant);

    CHECK_THROWS_AS(facade.startWorkflow(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.assignWork(nullptr, &item), std::invalid_argument);
    CHECK_THROWS_AS(facade.assignWork(&instance, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.completeWork(nullptr, &item), std::invalid_argument);
    CHECK_THROWS_AS(facade.completeWork(&instance, nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.escalateWork(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.closeWorkflow(nullptr), std::invalid_argument);
    CHECK_THROWS_AS(facade.assignWork(&instance, &item, candidates), std::logic_error);

    CHECK_THROWS_AS(facade.assignWork(nullptr, &item, candidates), std::invalid_argument);
    CHECK_THROWS_AS(facade.assignWork(&instance, nullptr, candidates), std::invalid_argument);
    CHECK_THROWS_AS(facade.completeWork(nullptr, &item, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.completeWork(&instance, nullptr, &participant), std::invalid_argument);
    CHECK_THROWS_AS(facade.completeWork(&instance, &item, nullptr), std::invalid_argument);
}
