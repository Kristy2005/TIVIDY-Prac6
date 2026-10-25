#include "engine/Participant.h"

namespace tividy {

Participant::Participant(const std::string& name, const Role& role)
    : name_(name), role_(role), workload_(0), available_(true) {}

void Participant::adjustWorkload(int delta) {
    workload_ += delta;
    if (workload_ < 0) workload_ = 0;
}

void Participant::setWorkload(int value) { workload_ = value < 0 ? 0 : value; }

} // namespace tividy
