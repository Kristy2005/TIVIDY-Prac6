#include "CancelWorkCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void CancelWorkCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("CancelWorkCommand: receiver is not set");
    saveState();
    receiver->close();
}
void CancelWorkCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("CancelWorkCommand: nothing to undo");
    receiver->restore(previous);
}
}
