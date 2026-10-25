#include "engine/Invoker.h"
#include "engine/Types.h"

namespace tividy {

void Invoker::execute(Command* command) {
    std::shared_ptr<Command> owned(command);
    if (!owned) throw WorkflowError("Cannot execute a null command");
    owned->execute();
    done_.push_back(owned);
    log_.push_back(owned->describe());
}

bool Invoker::canUndo() const { return !done_.empty() && done_.back()->isReversible(); }

void Invoker::undoLast() {
    if (done_.empty()) throw WorkflowError("Nothing to undo");
    if (!done_.back()->isReversible()) throw WorkflowError("The last action cannot be undone: " + done_.back()->describe());
    done_.back()->undo();
    log_.push_back("UNDO " + done_.back()->describe());
    done_.pop_back();
}

} // namespace tividy
