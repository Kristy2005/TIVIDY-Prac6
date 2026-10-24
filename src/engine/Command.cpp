#include "engine/Command.h"

namespace tividy {

Command::~Command() {}

InstanceCommand::InstanceCommand(WorkflowInstance& instance, const std::string& actor, bool reversible)
    : instance_(instance), actor_(actor), reversible_(reversible), executed_(false) {}

void InstanceCommand::execute() {
    if (executed_) throw WorkflowError("Command already executed: " + describe());
    if (reversible_) instance_.history().pushSnapshot(instance_.createMemento());
    try {
        doExecute();
    } catch (const std::exception& e) {
        // Roll back anything done before the failure, then leave a trace in the audit log.
        if (reversible_) instance_.restore(instance_.history().popSnapshot());
        instance_.recordEvent(EventType::InvalidAction, "", actor_, "", describe() + " rejected: " + e.what());
        throw;
    }
    executed_ = true;
}

void InstanceCommand::undo() {
    if (!reversible_) throw WorkflowError("Cannot undo: " + describe());
    if (!executed_) throw WorkflowError("Cannot undo a command that has not run: " + describe());
    instance_.restore(instance_.history().popSnapshot());
    executed_ = false;
    instance_.recordEvent(EventType::Undone, "", actor_, "", "Undid: " + describe());
}

} // namespace tividy
