#include "ApproveWorkCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void ApproveWorkCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("ApproveWorkCommand: receiver is not set");
    saveState();
    receiver->activate();
}
void ApproveWorkCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("ApproveWorkCommand: nothing to undo");
    receiver->restore(previous);
}
}
