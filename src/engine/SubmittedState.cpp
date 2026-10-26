#include "SubmittedState.h"
#include "ActiveState.h"
#include <stdexcept>

void SubmittedState::startProcessing(WorkflowInstance* instance) {
	throw std::logic_error("SUBMITTED STATE: Cannot start processing an inactive workflow instance");
}

void SubmittedState::close(WorkflowInstance* instance) {
	throw std::logic_error("SUBMITTED STATE: Cannot close an inactive workflow instance");
}

void SubmittedState::activate(WorkflowInstance* instance){
	if(instance != nullptr) {
		//TODO - DISCUSS CHANGES ON HOW NOTIFY OBESVERS WORKS 
		instance->setState(new ActiveState());
		instance->notifyObservers();
	}else{
		throw std::invalid_argument("SUBMITTED STATE: NULL PTR");
	}
}

void SubmittedState::resolve(WorkflowInstance* instance){
		throw std::logic_error("SUBMITTED STATE: Resolution not yet implemented");
}

