#include "WorkflowDirector.h"
#include <stdexcept>
namespace tividy {
WorkflowDirector::WorkflowDirector() : builder(nullptr) {}
void WorkflowDirector::setBuilder(WorkflowBuilder* value) { builder = value; }
void WorkflowDirector::constructWorkflow() {
    if (!builder) throw std::logic_error("WorkflowDirector: builder is not set");
    // Construction steps are client-specific; the configured builder is ready for use.
}
}
