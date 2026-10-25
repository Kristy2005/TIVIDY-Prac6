/**
 * @file Participant.h
 * @brief A person (or system) that can be given work.
 */
#ifndef TIVIDY_ENGINE_PARTICIPANT_H
#define TIVIDY_ENGINE_PARTICIPANT_H

#include <string>
#include "engine/Role.h"

namespace tividy {

/** @brief A named participant holding one Role, with a running workload count. */
class Participant {
public:
    Participant(const std::string& name, const Role& role);

    const std::string& name() const { return name_; }
    const Role& role() const { return role_; }
    Role& role() { return role_; }

    /** @return Number of work items currently assigned. */
    int workload() const { return workload_; }
    /** @brief Add @p delta to the workload; it never drops below zero. */
    void adjustWorkload(int delta);
    /** @brief Overwrite the workload (used when restoring a Memento). */
    void setWorkload(int value);

    /** @return false if the participant is away and must not receive work. */
    bool isAvailable() const { return available_; }
    void setAvailable(bool available) { available_ = available; }

private:
    std::string name_;
    Role role_;
    int workload_;
    bool available_;
};

} // namespace tividy

#endif
