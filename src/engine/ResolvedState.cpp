#include "ResolvedState.h"
#include <stdexcept>

void ResolvedState::startProcessing(WorkflowInstance* instance) {
	throw std::logic_error("RESOLVED STATE: Cannot start processing a resolved workflow instance");
}

void ResolvedState::close(WorkflowInstance* instance) {
	throw std::logic_error("RESOLVED STATE: Cannot close a resolved workflow instance");
}

void ResolvedState::activate(WorkflowInstance* instance){
	throw std::logic_error("RESOLVED STATE: Cannot activate a resolved workflow instance");
}

void ResolvedState::resolve(WorkflowInstance* instance){
	throw std::logic_error("RESOLVED STATE: Cannot resolve an already resolved workflow instance");
}