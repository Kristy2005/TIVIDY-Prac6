#include "PriorityDecorator.h"
#include <stdexcept>

namespace tividy {
PriorityDecorator::PriorityDecorator(WorkflowComponent* wrapped, int value):WorkItemDecorator(wrapped), priority(value) {}

int PriorityDecorator::getPriority() const {
    return priority;
}

void PriorityDecorator::setPriority(int value) {
    if (value < 0) {
        throw std::invalid_argument("PriorityDecorator: priority cannot be negative");
    }
    priority = value;
}

void PriorityDecorator::execute() {
    component->execute();
}

bool PriorityDecorator::isComplete() const {
    return component->isComplete();
}

WorkflowComponent* PriorityDecorator::clone() const {
    WorkflowComponent* wrappedCopy = component->clone();
    PriorityDecorator* copy = nullptr;
    try {
        copy = new PriorityDecorator(wrappedCopy, priority);
    } catch (...) {
        delete wrappedCopy;
        throw;
    }
    copy->setOwnsWrapped(true);
    return copy;
}

}