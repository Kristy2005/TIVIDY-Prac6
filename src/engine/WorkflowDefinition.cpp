#include "engine/WorkflowDefinition.h"
#include <algorithm>

namespace tividy {

WorkflowDefinition::WorkflowDefinition(const std::string& name)
    : name_(name), root_(new WorkflowStage("root", name)) {}

WorkflowDefinition::~WorkflowDefinition() {}

bool WorkflowDefinition::hasRole(const std::string& role) const {
    return std::find(roles_.begin(), roles_.end(), role) != roles_.end();
}

WorkflowComponent* WorkflowDefinition::createWorkTree() const { return root_->clone(); }

} // namespace tividy
