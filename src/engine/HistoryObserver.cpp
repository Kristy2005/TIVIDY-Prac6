#include "HistoryObserver.h"
#include "WorkflowInstance.h"
#include "WorkflowHistory.h"
#include <stdexcept>
namespace tividy {
void HistoryObserver::update(WorkflowInstance* instance) {
    if (!instance) throw std::invalid_argument("HistoryObserver: null instance");
    static WorkflowHistory history;
    history.save(instance->createMemento());
}
}
