#include "doctest.h"

#include <stdexcept>

#include "engine/Handler.h"
#include "engine/ApprovalHandler.h"
#include "engine/EscalationHandler.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/InProgressState.h"

using namespace tividy;

TEST_CASE("Chain of Responsibility handlers can be linked")
{
    ApprovalHandler approval;
    EscalationHandler escalation;
    FinalDecisionHandler finalDecision;

    CHECK_NOTHROW(
        approval.setSuccessor(&escalation)
    );

    CHECK_NOTHROW(
        escalation.setSuccessor(&finalDecision)
    );
}

TEST_CASE("Configured escalation chain handles a workflow request")
{
    ApprovalHandler approval;
    EscalationHandler escalation;
    FinalDecisionHandler finalDecision;

    approval.setSuccessor(&escalation);
    escalation.setSuccessor(&finalDecision);

    WorkflowDefinition definition;
    definition.setEscalationHandler(&approval);

    WorkflowInstance instance(&definition);

    static InProgressState inProgress;
    instance.setState(&inProgress);

    CHECK_NOTHROW(
        definition.handleEscalation(&instance)
    );
}

TEST_CASE("WorkflowDefinition rejects escalation when no chain is configured")
{
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);

    CHECK_THROWS_AS(
        definition.handleEscalation(&instance),
        std::logic_error
    );
}

TEST_CASE("WorkflowDefinition rejects a null escalation request")
{
    ApprovalHandler approval;

    WorkflowDefinition definition;
    definition.setEscalationHandler(&approval);

    CHECK_THROWS_AS(
        definition.handleEscalation(nullptr),
        std::invalid_argument
    );
}