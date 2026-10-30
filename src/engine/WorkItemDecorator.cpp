#include "WorkItemDecorator.h"
#include <stdexcept>

namespace tividy {
WorkItemDecorator::WorkItemDecorator(WorkflowComponent* wrapped) : component(wrapped) {
    if (component == nullptr) {
        throw std::invalid_argument("WorkItemDecorator requires a non-null component");
    }
}

void WorkItemDecorator::collectWorkItems(std::vector<WorkItem*>& items) {
    component->collectWorkItems(items);
}

WorkItemDecorator::~WorkItemDecorator() {
    // The wrapped component is non-owning and must not be deleted here.
    }
}
