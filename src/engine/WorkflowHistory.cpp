#include "WorkflowHistory.h"
#include <stdexcept>

namespace tividy {

void WorkflowHistory::save(WorkflowMemento* memento) {
	if(memento == nullptr) {
		throw std::invalid_argument("WorkflowHistory::save: memento cannot be null");
	}
	this->mementos.push_back(memento);
}

WorkflowMemento* WorkflowHistory::getLast() {
	if(this->mementos.empty()) {
		throw std::logic_error("WorkflowHistory::getLast: no saved mementos");
	}
	return this->mementos.back();
	}
}
