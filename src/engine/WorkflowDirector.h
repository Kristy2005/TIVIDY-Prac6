#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

#include <string>
#include <vector>

#include "WorkflowBuilder.h"

namespace tividy {

/**
 * @brief One step of a workflow recipe given to the WorkflowDirector.
 */
struct WorkflowStep {
    /** @brief Non-blank work item name. */
    std::string name;
    /** @brief Role required to perform the step; may be empty. */
    std::string role;
    /** @brief Non-negative priority; 0 means no priority decorator is added. */
    int priority;

    WorkflowStep(const std::string& stepName,
                 const std::string& stepRole = std::string(),
                 int stepPriority = 0)
        : name(stepName), role(stepRole), priority(stepPriority) {}
};

/**
 * @brief Builder pattern: Director. Knows the order of construction steps.
 *
 * The director drives any WorkflowBuilder through a fixed recipe so clients
 * do not repeat the same create/depend/prioritise calls. Steps are validated
 * before the builder is touched, so an invalid recipe builds nothing.
 *
 * Work items are made with WorkflowBuilder::createWorkItem(), so the
 * resulting definition owns them. The builder pointer is non-owning.
 */
class WorkflowDirector {
private:
    /** @brief Selected builder; not owned. */
    WorkflowBuilder* builder;

    /** @brief Steps collected with addStep(), used by constructWorkflow(). */
    std::vector<WorkflowStep> steps;

    /** @brief Check every step; throws std::invalid_argument on a bad one. */
    static void validate(const std::vector<WorkflowStep>& recipe);

public:
    /** @brief Construct a director without a builder. */
    WorkflowDirector();

    /**
     * @brief Select the builder that the director should use.
     * @param builder Builder to retain without taking ownership; may be null.
     */
    void setBuilder(WorkflowBuilder* builder);

    /**
     * @brief Append a step to the stored recipe used by constructWorkflow().
     * @param name Non-blank item name.
     * @param role Required role; may be empty.
     * @param priority Non-negative priority.
     * @throws std::invalid_argument If the step is invalid; nothing is stored.
     */
    void addStep(const std::string& name,
                 const std::string& role = std::string(),
                 int priority = 0);

    /** @brief Forget all stored steps. */
    void clearSteps();

    /**
     * @brief Build the stored recipe as a sequential workflow.
     *
     * Does nothing when no steps are stored.
     * @throws std::logic_error If no builder has been configured.
     */
    void constructWorkflow();

    /**
     * @brief Build steps that run one after another: each depends on the previous.
     * @param recipe Steps in order.
     * @throws std::logic_error If no builder has been configured.
     * @throws std::invalid_argument If a step is invalid (nothing is built).
     */
    void constructSequentialWorkflow(const std::vector<WorkflowStep>& recipe);

    /**
     * @brief Build steps with no dependencies between them.
     * @param recipe Steps in any order.
     * @throws std::logic_error If no builder has been configured.
     * @throws std::invalid_argument If a step is invalid (nothing is built).
     */
    void constructParallelWorkflow(const std::vector<WorkflowStep>& recipe);
};

}

#endif // WORKFLOWDIRECTOR_H