/**
 * @file Types.h
 * @brief Shared enumerations and exception types used across the generic TIVIDY engine.
 *
 * Nothing in this file (or anywhere in src/engine/) may mention a specific industry.
 */
#ifndef TIVIDY_ENGINE_TYPES_H
#define TIVIDY_ENGINE_TYPES_H

#include <ostream>
#include <stdexcept>
#include <string>

namespace tividy {

/** @brief How urgently a work item should be handled. */
enum class Priority { Normal = 0, High = 1, Urgent = 2 };

/** @brief Lifecycle of a single work item (a leaf of the workflow tree). */
enum class WorkStatus {
    Pending,    ///< Waiting for its dependencies / routing decision.
    Available,  ///< Ready to be worked on but nobody is responsible yet.
    Assigned,   ///< A participant is responsible for it.
    Completed,  ///< Finished, with an outcome.
    Skipped,    ///< A routing decision meant this branch was not taken.
    Cancelled   ///< Withdrawn, or a dependency was cancelled.
};

/** @return Human readable name of @p p. */
const char* priorityName(Priority p);
/** @return Human readable name of @p s. */
const char* statusName(WorkStatus s);
/** @brief Stream support (also lets doctest print the enums). */
std::ostream& operator<<(std::ostream& os, Priority p);
std::ostream& operator<<(std::ostream& os, WorkStatus s);

/** @return true if @p s can never change again (Completed, Skipped, Cancelled). */
bool isTerminal(WorkStatus s);

/** @brief Base class for every error the engine reports to its callers. */
class WorkflowError : public std::runtime_error {
public:
    explicit WorkflowError(const std::string& what) : std::runtime_error(what) {}
};

/** @brief Thrown when an action is not permitted in the current state/lifecycle position. */
class InvalidTransition : public WorkflowError {
public:
    explicit InvalidTransition(const std::string& what) : WorkflowError(what) {}
};

} // namespace tividy

#endif
