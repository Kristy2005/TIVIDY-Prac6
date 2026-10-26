#include "Participant.h"

namespace tividy{

  // Returns the index of target in items, or -1 if not found.
  int indexOfItem(const std::vector<WorkItem*>& items, WorkItem* target) {
      for (size_t i = 0; i < items.size(); ++i) {
          if (items[i] == target) {
              return static_cast<int>(i);
          }
      }
      return -1;
  }

  bool containsRole(const std::vector<std::string>& roles, const std::string& roleName) {
      for (size_t i = 0; i < roles.size(); ++i) {
          if (roles[i] == roleName) {
              return true;
          }
      }
      return false;
  }

  Participant::Participant() : name(), workload(0), assignedItems(), roles() {
  }

  Participant::Participant(const std::string& participantName)
      : name(participantName), workload(0), assignedItems(), roles() {
  }

  void Participant::assignWork(WorkItem* item) {
      if (item == nullptr || item->isComplete()) {
          return;
      }

      if (indexOfItem(assignedItems, item) != -1) {
          return;
      }

      assignedItems.push_back(item);
      ++workload;
  }

  void Participant::completeWork(WorkItem* item) {
      if (item == nullptr) {
          return;
      }

      int index = indexOfItem(assignedItems, item);
      if (index == -1) {
          return;
      }

      item->complete();
      assignedItems.erase(assignedItems.begin() + index);
      if (workload > 0) {
          --workload;
      }
  }

  int Participant::getWorkload() const {
      return workload;
  }

  const std::string& Participant::getName() const {
      return name;
  }

  void Participant::addRole(const std::string& roleName) {
      if (roleName.empty()) {
          return;
      }

      if (!containsRole(roles, roleName)) {
          roles.push_back(roleName);
      }
  }

  bool Participant::hasRole(const std::string& roleName) const {
      if (roleName.empty()) {
          return false;
      }

      return containsRole(roles, roleName);
  }

}