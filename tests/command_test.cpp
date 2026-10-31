#include "doctest.h"

#include <stdexcept>

#include "engine/ApproveWorkCommand.h"
#include "engine/AssignWorkCommand.h"
#include "engine/CancelWorkCommand.h"
#include "engine/CloseWorkflowCommand.h"
#include "engine/CompleteWorkCommand.h"
#include "engine/EscalateWorkCommand.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/Handler.h"
#include "engine/Invoker.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowState.h"
#include "engine/ActiveState.h"
#include "engine/InProgressState.h"
#include "engine/ResolvedState.h"
#include "engine/SubmittedState.h"
#include "engine/ClosedState.h"

using namespace tividy;

namespace {

class CountingFinalDecisionHandler : public FinalDecisionHandler {
public:
    CountingFinalDecisionHandler() : calls(0) {
    }

    virtual void handleRequest(WorkflowInstance* instance) override {
        ++calls;
        FinalDecisionHandler::handleRequest(instance);
    }

    int calls;
};

template <typename CommandType>
void checkCommandRejectsMissingReceiverAndSnapshot() {
    CommandType command;

    CHECK_THROWS_AS(command.execute(), std::logic_error);
    CHECK_THROWS_AS(command.undo(), std::logic_error);
}

void moveToActive(WorkflowInstance& instance) {
    instance.activate();
}

void moveToInProgress(WorkflowInstance& instance) {
    instance.activate();
    instance.startProcessing();
}

} // namespace

TEST_CASE("Invoker rejects execution and undo when no command is configured") {
    Invoker invoker;

    CHECK_THROWS_AS(invoker.executeCommand(), std::logic_error);
    CHECK_THROWS_AS(invoker.undoCommand(), std::logic_error);
}

TEST_CASE("Invoker executes and undoes the configured command") {
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);
    ApproveWorkCommand command;
    command.setReceiver(&instance);

    Invoker invoker;
    invoker.setCommand(&command);

    invoker.executeCommand();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);

    invoker.undoCommand();
    CHECK(dynamic_cast<SubmittedState*>(instance.getState()) != nullptr);
}

TEST_CASE("ApproveWorkCommand activates a submitted instance and undo restores Submitted") {
    WorkflowInstance instance;
    ApproveWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<SubmittedState*>(instance.getState()) != nullptr);
}

TEST_CASE("AssignWorkCommand starts processing and undo restores Active") {
    WorkflowInstance instance;
    moveToActive(instance);

    AssignWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(dynamic_cast<InProgressState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);
}

TEST_CASE("CompleteWorkCommand resolves an in-progress instance and undo restores it") {
    WorkflowInstance instance;
    moveToInProgress(instance);

    CompleteWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(dynamic_cast<ResolvedState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<InProgressState*>(instance.getState()) != nullptr);
}

TEST_CASE("CancelWorkCommand closes an active instance and undo restores Active") {
    WorkflowInstance instance;
    moveToActive(instance);

    CancelWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(dynamic_cast<ClosedState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);
}

TEST_CASE("CloseWorkflowCommand closes an active instance and undo restores Active") {
    WorkflowInstance instance;
    moveToActive(instance);

    CloseWorkflowCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(dynamic_cast<ClosedState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);
}

TEST_CASE("EscalateWorkCommand forwards escalation without changing the workflow state") {
    WorkflowDefinition definition;
    CountingFinalDecisionHandler finalDecision;
    definition.setEscalationHandler(&finalDecision);

    WorkflowInstance instance(&definition);
    moveToInProgress(instance);

    EscalateWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    CHECK(finalDecision.calls == 1);
    CHECK(dynamic_cast<InProgressState*>(instance.getState()) != nullptr);

    command.undo();
    CHECK(dynamic_cast<InProgressState*>(instance.getState()) != nullptr);
}

TEST_CASE("Concrete commands reject execute without a receiver and undo without a snapshot") {
    checkCommandRejectsMissingReceiverAndSnapshot<ApproveWorkCommand>();
    checkCommandRejectsMissingReceiverAndSnapshot<AssignWorkCommand>();
    checkCommandRejectsMissingReceiverAndSnapshot<CompleteWorkCommand>();
    checkCommandRejectsMissingReceiverAndSnapshot<CancelWorkCommand>();
    checkCommandRejectsMissingReceiverAndSnapshot<CloseWorkflowCommand>();
    checkCommandRejectsMissingReceiverAndSnapshot<EscalateWorkCommand>();
}

TEST_CASE("A command can be executed again after its previous snapshot has been undone") {
    WorkflowInstance instance;
    ApproveWorkCommand command;
    command.setReceiver(&instance);

    command.execute();
    command.undo();
    CHECK(dynamic_cast<SubmittedState*>(instance.getState()) != nullptr);

    command.execute();
    CHECK(dynamic_cast<ActiveState*>(instance.getState()) != nullptr);
}
