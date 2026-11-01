#include "WorkflowHistory.h"

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

std::vector<std::string> WorkflowHistory::getHistory() const {
	std::vector<std::string> history;

	for (const auto& memento : mementos) {
		if (memento == nullptr || memento->getState() == nullptr) {
			continue;
		}

		std::time_t t = memento->getTimestamp();
		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

		WorkflowState* state = memento->getState();
		std::ostringstream line;

		std::string label;

		if (dynamic_cast<SubmittedState*>(state) != nullptr) {
			label = "Submitted";
		} else if (dynamic_cast<ActiveState*>(state) != nullptr) {
			label = "Active";
		} else if (dynamic_cast<InProgressState*>(state) != nullptr) {
			label = "In Progress";
		} else if (dynamic_cast<ResolvedState*>(state) != nullptr) {
			label = "Resolved";
		} else if (dynamic_cast<ClosedState*>(state) != nullptr) {
			label = "Closed";
		} else {
			label = "Unknown";
		}

		line << "[" << buffer << "]" << "State :" << label;
		history.push_back(line.str());
		
	}

	return history;
}

}
