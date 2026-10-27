#include "EscalateWorkCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void EscalateWorkCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("EscalateWorkCommand: receiver is not set");
    saveState();
    receiver->escalate();
}
void EscalateWorkCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("EscalateWorkCommand: nothing to undo");
    receiver->restore(previous);
}
}
