#include "ActiveState.h"
#include "InProgressState.h"
#include "ClosedState.h"
#include <stdexcept>

void ActiveState::startProcessing(WorkflowInstance* instance) {
	if(instance != nullptr) {
		//TODO - DISCUSS CHANGES ON HOW NOTIFY OBESVERS WORKS 
		instance->setState(new InProgressState());
		instance->notifyObservers();
	}else{
		throw std::invalid_argument("ACTIVE STATE: NULL PTR");
	}
}

void ActiveState::close(WorkflowInstance* instance) {
	if(instance != nullptr) {
		//TODO - DISCUSS CHANGES ON HOW NOTIFY OBESVERS WORKS 
		instance->setState(new ClosedState());
		instance->notifyObservers();
	}else{	
		throw std::invalid_argument("ACTIVE STATE: NULL PTR");
	}
}

void ActiveState::activate(WorkflowInstance* instance){
	throw std::logic_error("ACTIVESTATE: Cannot activate an already active workflow instance");
}

void ActiveState::resolve(WorkflowInstance* instance){
	
	throw std::logic_error("ACTIVESTATE: Cannot resolve an inactive workflow instance");
}

