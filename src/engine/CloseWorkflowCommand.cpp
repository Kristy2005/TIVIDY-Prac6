#include "CloseWorkflowCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void CloseWorkflowCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("CloseWorkflowCommand: receiver is not set");
    saveState();
    receiver->close();
}
void CloseWorkflowCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("CloseWorkflowCommand: nothing to undo");
    receiver->restore(previous);
}
}
