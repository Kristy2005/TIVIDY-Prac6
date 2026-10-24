#include "engine/WorkflowObserver.h"
#include <algorithm>

namespace tividy {

WorkflowObserver::~WorkflowObserver() {}
WorkflowSubject::~WorkflowSubject() {}

void WorkflowSubject::attach(WorkflowObserver* o) {
    if (!o) return;
    if (std::find(observers_.begin(), observers_.end(), o) == observers_.end()) observers_.push_back(o);
}

void WorkflowSubject::detach(WorkflowObserver* o) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), o), observers_.end());
}

void WorkflowSubject::notify(const WorkflowEvent& event) {
    // Copy so an observer may attach/detach while being notified.
    std::vector<WorkflowObserver*> snapshot(observers_);
    for (size_t i = 0; i < snapshot.size(); ++i) snapshot[i]->update(event);
}

} // namespace tividy
