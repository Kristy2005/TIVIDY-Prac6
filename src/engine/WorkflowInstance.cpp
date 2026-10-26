#include "WorkflowInstance.h"
#include "WorkflowDefinition.h"
#include <stdexcept>
#include "SubmittedState.h"
#include "WorkflowMemento.h"
#include "WorkflowObserver.h"
#include <algorithm>

namespace tividy {

	void WorkflowInstance::setState(WorkflowState* state) {
		if(state == nullptr) {
			throw std::invalid_argument("WorkflowInstance::setState: state cannot be null");
		}
		
		if(this->currentState)delete this->currentState;
		this->currentState = state;
	}

	void WorkflowInstance::submit() {
		this->currentState = new SubmittedState();
	}

	void WorkflowInstance::activate(){
		this->currentState->activate(this);
	}

	void WorkflowInstance::startProcessing() {
		this->currentState->startProcessing(this);
	}

	void WorkflowInstance::escalate() {
		this->definition->handleEscalation(this);
	}

	WorkflowMemento* WorkflowInstance::createMemento() {
		WorkflowMemento* memento = new WorkflowMemento();
		memento->setState(this->currentState);
		return memento;
	}

	void WorkflowInstance::restore(WorkflowMemento* memento) {
		if(memento == nullptr) {
			throw std::invalid_argument("WorkflowInstance::restore: memento cannot be null");
		}
		this->setState(memento->getState());
	}

	void WorkflowInstance::resolve() {
		this->currentState->resolve(this);
	}

	void WorkflowInstance::close() {
		this->currentState->close(this);
	}

	void WorkflowInstance::notifyObservers() {

		if(this->observers.empty()) {
			throw std::logic_error("WorkflowInstance::notifyObservers: no registered observers");
		}

		for(WorkflowObserver* observer : observers) {
			observer->update(this);
		}
	}

	void WorkflowInstance::attach(WorkflowObserver* observer) {
		this->observers.push_back(observer);
	}

	void WorkflowInstance::detach(WorkflowObserver* observer) {
		auto it = std::find(observers.begin(), observers.end(), observer);
		if(it != observers.end()) {
			observers.erase(it);
		} else {
			throw std::invalid_argument("WorkflowInstance::detach: observer not found");
		}
	}
}
