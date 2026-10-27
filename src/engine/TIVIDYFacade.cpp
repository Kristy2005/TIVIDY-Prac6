#include "TIVIDYFacade.h"
#include "WorkItem.h"
#include <stdexcept>
namespace tividy {
WorkflowDefinition* TIVIDYFacade::createWorkflow() { return new WorkflowDefinition(); }
WorkflowInstance* TIVIDYFacade::startWorkflow(WorkflowDefinition* definition) {
    if (!definition) throw std::invalid_argument("startWorkflow: definition cannot be null");
    WorkflowInstance* instance = definition->createInstance();
    instance->submit();
    instance->activate();
    return instance;
}
void TIVIDYFacade::assignWork(WorkflowInstance* instance, WorkItem* item) {
    if (!instance || !item) throw std::invalid_argument("assignWork: null argument");
    item->execute();
}
void TIVIDYFacade::completeWork(WorkflowInstance* instance, WorkItem* item) {
    if (!instance || !item) throw std::invalid_argument("completeWork: null argument");
    item->complete();
    instance->resolve();
}
void TIVIDYFacade::escalateWork(WorkflowInstance* instance) {
    if (!instance) throw std::invalid_argument("escalateWork: instance cannot be null");
    instance->escalate();
}
void TIVIDYFacade::closeWorkflow(WorkflowInstance* instance) {
    if (!instance) throw std::invalid_argument("closeWorkflow: instance cannot be null");
    instance->close();
}
}
