#include "WorkloadBasedAssignment.h"
#include "Participant.h"
#include "WorkItem.h"

namespace tividy{
  Participant* WorkloadBasedAssignment::assign(
      WorkItem* item, std::vector<Participant*> participants) {
      if (item == nullptr || item->isComplete()) {
          return nullptr;
      }

      Participant* selected = nullptr;
      for (std::vector<Participant*>::const_iterator it = participants.begin();
           it != participants.end(); ++it) {
          Participant* candidate = *it;
          if (candidate == nullptr) {
              continue;
          }

          if (selected == nullptr || candidate->getWorkload() < selected->getWorkload()) {
              selected = candidate;
          }
      }

      return selected;
  }
  }
}