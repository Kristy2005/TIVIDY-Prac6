#include "AssignWorkCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void AssignWorkCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("AssignWorkCommand: receiver is not set");
    saveState();
    receiver->startProcessing();
}
void AssignWorkCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("AssignWorkCommand: nothing to undo");
    receiver->restore(previous);
}
}
