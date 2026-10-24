/**
 * @file ProgressObserver.h
 * @brief Observer that keeps a live progress figure for an instance.
 */
#ifndef TIVIDY_ENGINE_PROGRESSOBSERVER_H
#define TIVIDY_ENGINE_PROGRESSOBSERVER_H

#include <string>
#include "engine/WorkflowObserver.h"

namespace tividy {

class WorkflowInstance;

/** @brief Recomputes "finished / total" from the instance after every event (so undo is reflected too). */
class ProgressObserver : public WorkflowObserver {
public:
    explicit ProgressObserver(const WorkflowInstance& instance);
    void update(const WorkflowEvent& event);

    int finished() const { return finished_; }
    int total() const { return total_; }
    /** @return 0..100. */
    int percent() const { return total_ == 0 ? 100 : (finished_ * 100) / total_; }
    /** @return Text such as "3/5 (60%)". */
    std::string summary() const;
    /** @return The type of the most recent event seen. */
    EventType lastEvent() const { return last_; }

private:
    const WorkflowInstance& instance_;
    int finished_;
    int total_;
    EventType last_;
};

} // namespace tividy

#endif
