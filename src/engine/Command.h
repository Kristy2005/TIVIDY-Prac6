/**
 * @file Command.h
 * @brief Command pattern: every action on a workflow instance is an object.
 *
 * Commands can be logged, queued and undone. Reversible commands take a Memento of the
 * instance before they run, so a failure part-way through rolls the instance back, and
 * undo() restores the snapshot.
 */
#ifndef TIVIDY_ENGINE_COMMAND_H
#define TIVIDY_ENGINE_COMMAND_H

#include <string>
#include "engine/WorkflowInstance.h"

namespace tividy {

/** @brief Command interface. */
class Command {
public:
    virtual ~Command();
    /** @brief Perform the action. Throws WorkflowError if it is not allowed (the instance is left unchanged). */
    virtual void execute() = 0;
    /** @brief Reverse a successfully executed command. Throws if irreversible or not executed. */
    virtual void undo() = 0;
    virtual bool isReversible() const = 0;
    /** @return One-line description, e.g. "Complete review by bob". */
    virtual std::string describe() const = 0;
};

/**
 * @brief Template-method base for commands that act on one WorkflowInstance.
 * Subclasses implement doExecute(); this class handles snapshots, rollback, auditing and undo.
 */
class InstanceCommand : public Command {
public:
    void execute();
    void undo();
    bool isReversible() const { return reversible_; }

protected:
    InstanceCommand(WorkflowInstance& instance, const std::string& actor, bool reversible);
    virtual void doExecute() = 0;

    WorkflowInstance& instance_;
    std::string actor_;

private:
    bool reversible_;
    bool executed_;
};

/** @brief Start the workflow. */
class StartWorkflowCommand : public InstanceCommand {
public:
    StartWorkflowCommand(WorkflowInstance& w, const std::string& actor) : InstanceCommand(w, actor, true) {}
    std::string describe() const { return "Start workflow by " + actor_; }
protected:
    void doExecute() { instance_.start(actor_); }
};

/** @brief Assign a work item (to a named participant, or let the strategy choose if empty). */
class AssignWorkCommand : public InstanceCommand {
public:
    AssignWorkCommand(WorkflowInstance& w, const std::string& itemId, const std::string& participant,
                      const std::string& actor)
        : InstanceCommand(w, actor, true), itemId_(itemId), participant_(participant) {}
    std::string describe() const { return "Assign " + itemId_ + " to " + (participant_.empty() ? "<strategy>" : participant_); }
protected:
    void doExecute() { instance_.assign(itemId_, participant_, actor_); }
private:
    std::string itemId_, participant_;
};

/** @brief Complete a work item with an outcome (the outcome drives decision branches). */
class CompleteWorkCommand : public InstanceCommand {
public:
    CompleteWorkCommand(WorkflowInstance& w, const std::string& itemId, const std::string& actor,
                        const std::string& outcome = "done")
        : InstanceCommand(w, actor, true), itemId_(itemId), outcome_(outcome) {}
    std::string describe() const { return "Complete " + itemId_ + " by " + actor_ + " (" + outcome_ + ")"; }
protected:
    void doExecute() { instance_.complete(itemId_, actor_, outcome_); }
private:
    std::string itemId_, outcome_;
};

/** @brief Escalate the case to a higher authority via the Chain of Responsibility. */
class EscalateWorkCommand : public InstanceCommand {
public:
    EscalateWorkCommand(WorkflowInstance& w, const std::string& actor, const std::string& reason)
        : InstanceCommand(w, actor, true), reason_(reason) {}
    std::string describe() const { return "Escalate by " + actor_ + ": " + reason_; }
protected:
    void doExecute() { instance_.escalate(actor_, reason_); }
private:
    std::string reason_;
};

/** @brief The escalation handler approves/resolves the escalation, returning the case to InProgress. */
class ApproveWorkCommand : public InstanceCommand {
public:
    ApproveWorkCommand(WorkflowInstance& w, const std::string& actor, const std::string& notes)
        : InstanceCommand(w, actor, true), notes_(notes) {}
    std::string describe() const { return "Resolve escalation by " + actor_; }
protected:
    void doExecute() { instance_.resolveEscalation(actor_, notes_); }
private:
    std::string notes_;
};

/** @brief Withdraw a single work item (dependants are cancelled too). */
class CancelWorkCommand : public InstanceCommand {
public:
    CancelWorkCommand(WorkflowInstance& w, const std::string& itemId, const std::string& actor, const std::string& reason)
        : InstanceCommand(w, actor, true), itemId_(itemId), reason_(reason) {}
    std::string describe() const { return "Cancel " + itemId_ + " by " + actor_; }
protected:
    void doExecute() { instance_.cancelItem(itemId_, actor_, reason_); }
private:
    std::string itemId_, reason_;
};

/** @brief Formally close the whole workflow (e.g. withdrawn or final appeal denied). Irreversible. */
class CloseWorkflowCommand : public InstanceCommand {
public:
    CloseWorkflowCommand(WorkflowInstance& w, const std::string& actor, const std::string& reason)
        : InstanceCommand(w, actor, false), reason_(reason) {}
    std::string describe() const { return "Close workflow by " + actor_ + ": " + reason_; }
protected:
    void doExecute() { instance_.close(actor_, reason_); }
private:
    std::string reason_;
};

} // namespace tividy

#endif
