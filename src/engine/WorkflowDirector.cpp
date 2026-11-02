#include "WorkflowDirector.h"
#include <stdexcept>
namespace tividy {

WorkflowDirector::WorkflowDirector() : builder(nullptr), steps() {}

void WorkflowDirector::setBuilder(WorkflowBuilder* value) { builder = value; }

void WorkflowDirector::validate(const std::vector<WorkflowStep>& recipe) {
    for (std::size_t i = 0; i < recipe.size(); ++i) {
        if (recipe[i].name.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument("WorkflowDirector: step name cannot be empty");
        }
        if (recipe[i].priority < 0) {
            throw std::invalid_argument("WorkflowDirector: step priority cannot be negative");
        }
    }
}

void WorkflowDirector::addStep(const std::string& name, const std::string& role, int priority) {
    std::vector<WorkflowStep> single(1, WorkflowStep(name, role, priority));
    validate(single);
    steps.push_back(single[0]);
}

void WorkflowDirector::clearSteps() { steps.clear(); }

void WorkflowDirector::constructWorkflow() {
    if (!builder) throw std::logic_error("WorkflowDirector: builder is not set");
    if (steps.empty()) return;
    constructSequentialWorkflow(steps);
}

void WorkflowDirector::constructSequentialWorkflow(const std::vector<WorkflowStep>& recipe) {
    if (!builder) throw std::logic_error("WorkflowDirector: builder is not set");
    validate(recipe);

    WorkItem* previous = nullptr;
    for (std::size_t i = 0; i < recipe.size(); ++i) {
        WorkItem* item = builder->createWorkItem(recipe[i].name, recipe[i].role);
        if (recipe[i].priority > 0) {
            builder->addPriority(item, recipe[i].priority);
        }
        if (previous != nullptr) {
            builder->addDependency(previous, item);
        }
        previous = item;
    }
}

void WorkflowDirector::constructParallelWorkflow(const std::vector<WorkflowStep>& recipe) {
    if (!builder) throw std::logic_error("WorkflowDirector: builder is not set");
    validate(recipe);

    for (std::size_t i = 0; i < recipe.size(); ++i) {
        WorkItem* item = builder->createWorkItem(recipe[i].name, recipe[i].role);
        if (recipe[i].priority > 0) {
            builder->addPriority(item, recipe[i].priority);
        }
    }
}

}