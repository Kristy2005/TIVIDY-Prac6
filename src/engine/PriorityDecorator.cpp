#include "PriorityDecorator.h"

namespace tividy {
PriorityDecorator::PriorityDecorator(WorkflowComponent* wrapped, int value) : WorkItemDecorator(wrapped), priority(value){}
int PriorityDecorator::getPriority() const {
    return priority;
}
void PriorityDecorator::execute(){
    component->execute();
}
bool PriorityDecorator::isComplete() const {
    return component->isComplete();
}
WorkflowComponent* PriorityDecorator::clone() const
{
    return new PriorityDecorator(component->clone(), priority);
}
}
