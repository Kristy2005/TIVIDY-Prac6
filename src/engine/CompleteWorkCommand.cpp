#include "CompleteWorkCommand.h"
#include "WorkflowInstance.h"
#include <stdexcept>
namespace tividy {
void CompleteWorkCommand::execute() {
    if (receiver == nullptr) throw std::logic_error("CompleteWorkCommand: receiver is not set");
    saveState();
    receiver->resolve();
}
void CompleteWorkCommand::undo() {
    if (receiver == nullptr || previous == nullptr) throw std::logic_error("CompleteWorkCommand: nothing to undo");
    receiver->restore(previous);
}
}
