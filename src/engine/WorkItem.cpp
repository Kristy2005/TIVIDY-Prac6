#include "WorkItem.h"
#include <iostream>
#include <stdexcept>

namespace tividy{
  WorkItem::WorkItem() : name(), completed(false), requiredRole(), priority(0) {
  }

  WorkItem::WorkItem(const std::string& itemName) : name(itemName), completed(false), requiredRole(), priority(0) {
  }

  WorkItem::WorkItem(const std::string& itemName, const std::string& requiredRoleName) : name(itemName), completed(false), requiredRole(requiredRoleName), priority(0) {
  }

  void WorkItem::execute() {
    std::cout<<"Executing work item: " << this->name << std::endl;
  }

  bool WorkItem::isComplete() const {
      return completed;
  }

  void WorkItem::complete() {
      completed = true;
  }

  const std::string& WorkItem::getName() const {
      return name;
  }

  void WorkItem::setRequiredRole(const std::string& requiredRoleName) {
      requiredRole = requiredRoleName;
  }

  const std::string& WorkItem::getRequiredRole() const {
      return requiredRole;
  }

  void WorkItem::setPriority(int value) {
      if (value < 0) {
          throw std::invalid_argument("WorkItem priority cannot be negative");
      }
      priority = value;
  }

  int WorkItem::getPriority() const {
      return priority;
  }

  void WorkItem::collectWorkItems(std::vector<WorkItem*>& items) {
    items.push_back(this);
}

WorkflowComponent* WorkItem::clone() const {
    return new WorkItem(*this);
}
void WorkItem::addDependency(WorkItem* item)
{
    if (item == nullptr || item == this) {
        return;
    }

    for (std::size_t i = 0; i < dependencies.size(); ++i) {
        if (dependencies[i] == item) {
            return;
        }
    }

    dependencies.push_back(item);
}

const std::vector<WorkItem*>& WorkItem::getDependencies() const
{
    return dependencies;
}
}
