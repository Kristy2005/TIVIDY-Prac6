#include "WorkItem.h"
#include <iostream>

namespace tividy{
  WorkItem::WorkItem() : name(), completed(false), requiredRole() {
  }

  WorkItem::WorkItem(const std::string& itemName) : name(itemName), completed(false), requiredRole() {
  }

  WorkItem::WorkItem(const std::string& itemName, const std::string& requiredRoleName) : name(itemName), completed(false), requiredRole(requiredRoleName) {
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
}
