#include "InProgressState.h"
#include "ResolvedState.h"
#include "ClosedState.h"
#include <stdexcept>

namespace tividy {

void InProgressState::startProcessing(WorkflowInstance* instance) {
	throw std::logic_error("IN PROGRESS STATE: Cannot start processing an already active workflow instance");
}

void InProgressState::close(WorkflowInstance* instance) {
	throw std::logic_error("IN PROGRESS STATE: Cannot close an in-progress workflow instance");
}

void InProgressState::activate(WorkflowInstance* instance){
	throw std::logic_error("IN PROGRESS STATE: Cannot activate an in-progress workflow instance");
}


void InProgressState::resolve(WorkflowInstance* instance){
	if(instance != nullptr) {
		//TODO - DISCUSS CHANGES ON HOW NOTIFY OBESVERS WORKS 
		instance->setState(new ResolvedState());
	}else{
		throw std::invalid_argument("IN PROGRESS STATE: NULL PTR");
	}
}


} // namespace tividy
