#include "InProgressState.h"
#include "ResolvedState.h"
#include "ClosedState.h"
#include <stdexcept>

namespace tividy {

void InProgressState::startProcessing(WorkflowInstance* instance) {
	throw std::logic_error("IN PROGRESS STATE: Cannot start processing an already active workflow instance");
}

void InProgressState::close(WorkflowInstance* instance) {
	if (instance == nullptr) {
		throw std::invalid_argument("IN PROGRESS STATE: NULL PTR");
	}
	static ClosedState s;
	instance->setState(&s);       // retracted or rejected while in progress
}

void InProgressState::activate(WorkflowInstance* instance){
	throw std::logic_error("IN PROGRESS STATE: Cannot activate an in-progress workflow instance");
}


void InProgressState::resolve(WorkflowInstance* instance) {
	if (instance == nullptr) {
		throw std::invalid_argument("IN PROGRESS STATE: NULL PTR");
	}
	static ResolvedState s;
	instance->setState(&s);     // setState() notifies observers
}


} // namespace tividy
