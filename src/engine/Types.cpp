#include "engine/Types.h"

namespace tividy {

const char* priorityName(Priority p) {
    switch (p) {
        case Priority::Normal: return "Normal";
        case Priority::High:   return "High";
        case Priority::Urgent: return "Urgent";
    }
    return "Normal";
}

const char* statusName(WorkStatus s) {
    switch (s) {
        case WorkStatus::Pending:   return "Pending";
        case WorkStatus::Available: return "Available";
        case WorkStatus::Assigned:  return "Assigned";
        case WorkStatus::Completed: return "Completed";
        case WorkStatus::Skipped:   return "Skipped";
        case WorkStatus::Cancelled: return "Cancelled";
    }
    return "Pending";
}

std::ostream& operator<<(std::ostream& os, Priority p) { return os << priorityName(p); }
std::ostream& operator<<(std::ostream& os, WorkStatus s) { return os << statusName(s); }

bool isTerminal(WorkStatus s) {
    return s == WorkStatus::Completed || s == WorkStatus::Skipped || s == WorkStatus::Cancelled;
}

} // namespace tividy
