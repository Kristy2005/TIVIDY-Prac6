#include "WorkItem.h"

WorkItem::WorkItem() : name(), completed(false) {}

WorkItem::WorkItem(const std::string& itemName) : name(itemName), completed(false) {}

void WorkItem::execute() {}

bool WorkItem::isComplete() const {
    return completed;
}

void WorkItem::complete() {
    completed = true;
}

const std::string& WorkItem::getName() const {
    return name;
}
