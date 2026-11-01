#ifndef TIVIDY_INTERACTIVE_WORKFLOW_SESSION_H
#define TIVIDY_INTERACTIVE_WORKFLOW_SESSION_H

#include <string>
#include <vector>

namespace tividy {
class Participant;
class TIVIDYFacade;
class WorkflowDefinition;
class WorkflowInstance;

/**
 * @brief Run the live, instance-management menu used by both industry demos.
 *
 * The supplied first instance is managed and deleted by this function. The
 * definition, facade and initial participant objects remain owned by the
 * calling industry scenario and must outlive this call.
 */
void runInteractiveWorkflowSession(
    TIVIDYFacade& facade,
    WorkflowDefinition* definition,
    WorkflowInstance* firstInstance,
    const std::vector<Participant*>& initialParticipants,
    const std::string& caseDescription
);

} // namespace tividy

#endif // TIVIDY_INTERACTIVE_WORKFLOW_SESSION_H
