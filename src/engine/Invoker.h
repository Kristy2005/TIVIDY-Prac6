/**
 * @file Invoker.h
 * @brief Command invoker: runs commands and remembers them for undo.
 */
#ifndef TIVIDY_ENGINE_INVOKER_H
#define TIVIDY_ENGINE_INVOKER_H

#include <memory>
#include <string>
#include <vector>
#include "engine/Command.h"

namespace tividy {

/** @brief Executes commands for one instance and keeps the undo stack (last in, first out). */
class Invoker {
public:
    /**
     * @brief Run @p command (takes ownership). If it throws, it is discarded and the error propagates.
     */
    void execute(Command* command);
    /** @return true if the most recent command can be undone. */
    bool canUndo() const;
    /** @brief Undo the most recent command. Throws WorkflowError if there is none or it is irreversible. */
    void undoLast();
    /** @return Descriptions of every command run (and undone), oldest first. */
    const std::vector<std::string>& log() const { return log_; }

private:
    std::vector<std::shared_ptr<Command> > done_;
    std::vector<std::string> log_;
};

} // namespace tividy

#endif
