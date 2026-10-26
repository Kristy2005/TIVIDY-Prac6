#include "ClosedState.h"
#include <stdexcept>

namespace tividy {
	void ClosedState::startProcessing(WorkflowInstance* instance) {
		throw std::logic_error("CLOSED STATE: Cannot start processing a closed workflow instance");
	}
	void ClosedState::close(WorkflowInstance* instance) {
		throw std::logic_error("CLOSED STATE: Cannot close an already closed workflow instance");
	}
	void ClosedState::activate(WorkflowInstance* instance){
		throw std::logic_error("CLOSED STATE: Cannot activate a closed workflow instance");
	}
	void ClosedState::resolve(WorkflowInstance* instance){
		throw std::logic_error("CLOSED STATE: Cannot resolve a closed workflow instance");
	}
} // namespace tividy
