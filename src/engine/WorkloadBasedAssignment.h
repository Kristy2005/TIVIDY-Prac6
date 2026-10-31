#ifndef TIVIDY_WORKLOAD_BASED_ASSIGNMENT_H
#define TIVIDY_WORKLOAD_BASED_ASSIGNMENT_H

#include "AssignmentStrategy.h"

namespace tividy{

  class WorkflowInstance;

  /**
   * @brief Assignment strategy that selects the participant with the smallest workload.
   *
   * Ties are resolved by preserving the order of the supplied candidates.
   * Selecting a participant does not itself record an assignment; the caller
   * should call Participant::assignWork once the selection is accepted.
   */
  class WorkloadBasedAssignment : public AssignmentStrategy {
  public:
      /**
       * @brief Select the least-loaded non-null participant for an incomplete item.
       * @param item Work item to assign; null or completed items are rejected.
       * @param participants Candidate participants; null entries are ignored.
       * @return Participant with the smallest workload, or null when no candidate remains.
       */      
      virtual Participant* assign(
        WorkItem* item,
        std::vector<Participant*> participants
    );

    virtual Participant* assign(
        WorkflowInstance* instance,
        WorkItem* item,
        std::vector<Participant*> participants
    );
  };
}
#endif // TIVIDY_WORKLOAD_BASED_ASSIGNMENT_H
