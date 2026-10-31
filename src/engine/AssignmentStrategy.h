#ifndef TIVIDY_ASSIGNMENT_STRATEGY_H
#define TIVIDY_ASSIGNMENT_STRATEGY_H

#include <vector>

#include "Participant.h"

namespace tividy {
  class Participant;
  class WorkItem;
  class WorkflowInstance;

  /**
   * @brief Interface for interchangeable work-assignment algorithms.
   *
   * Concrete strategies select a participant for a work item without changing
   * the work item or the workflow engine. The caller is responsible for
   * recording the assignment with Participant::assignWork after a participant
   * has been selected.
   */
  class AssignmentStrategy {
  public:
      /** @brief Virtual destructor. */
      virtual ~AssignmentStrategy() {}

      /**
       * @brief Select the participant who should receive a work item.
       * @param item Work item to route. A null item is rejected.
       * @param participants Candidates to consider; null entries are ignored.
       * @return Selected participant, or nullptr when no candidate is suitable.
       */
      virtual Participant* assign(WorkItem* item, std::vector<Participant*> participants) = 0;

      // Instance-aware assignment interface
    virtual Participant* assign(
        WorkflowInstance* instance,
        WorkItem* item,
        std::vector<Participant*> participants
    ) {
        (void)instance;
        (void)item;
        (void)participants;
        return nullptr;
    }
   };
}
#endif 
