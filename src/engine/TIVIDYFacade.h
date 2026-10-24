/**
 * @file TIVIDYFacade.h
 * @brief Facade pattern: the one entry point a UI or scenario layer uses to talk to the engine.
 */
#ifndef TIVIDY_ENGINE_TIVIDYFACADE_H
#define TIVIDY_ENGINE_TIVIDYFACADE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "engine/Invoker.h"
#include "engine/NotificationObserver.h"
#include "engine/NotificationService.h"
#include "engine/WorkflowInstance.h"

namespace tividy {

/**
 * @brief Hides definitions, instances, commands, observers and notifications behind a few calls.
 *
 * Every mutating call builds a Command and runs it through the instance's Invoker, so all actions
 * are audited, rolled back on failure, and (where reversible) undoable. Errors are reported as
 * WorkflowError / InvalidTransition exceptions; the instance is left unchanged when that happens.
 */
class TIVIDYFacade {
public:
    /** @param notifier Channel for notifications (not owned; may be NULL for none). */
    explicit TIVIDYFacade(NotificationService* notifier = 0);

    /** @brief Change the notification channel used by instances created from now on. */
    void setNotificationService(NotificationService* notifier);

    // ---- definitions ---------------------------------------------------------------------------
    /** @brief Make a built definition available for starting instances. Replaces one with the same name. */
    void registerWorkflow(std::shared_ptr<const WorkflowDefinition> definition);
    std::vector<std::string> workflowNames() const;

    // ---- instances -----------------------------------------------------------------------------
    /** @brief Create (but do not start) an independent instance of a registered workflow. @return Its id. */
    std::string createInstance(const std::string& workflowName);
    void addParticipant(const std::string& instanceId, const std::string& name, const std::string& role);
    void setData(const std::string& instanceId, const std::string& key, const std::string& value);
    std::vector<std::string> instanceIds() const;

    // ---- actions -------------------------------------------------------------------------------
    void startWorkflow(const std::string& instanceId, const std::string& actor = "system");
    /** @brief Assign an item. Leave @p participant empty to let the definition's strategy choose. */
    void assignWork(const std::string& instanceId, const std::string& itemId, const std::string& participant,
                    const std::string& actor);
    void completeWork(const std::string& instanceId, const std::string& itemId, const std::string& actor,
                      const std::string& outcome = "done");
    void escalateWork(const std::string& instanceId, const std::string& actor, const std::string& reason);
    void approveEscalation(const std::string& instanceId, const std::string& actor, const std::string& notes);
    void cancelWork(const std::string& instanceId, const std::string& itemId, const std::string& actor,
                    const std::string& reason);
    void closeWorkflow(const std::string& instanceId, const std::string& actor, const std::string& reason);
    /** @brief Undo the last reversible action on this instance. */
    void undo(const std::string& instanceId);
    bool canUndo(const std::string& instanceId) const;

    // ---- inspection ----------------------------------------------------------------------------
    WorkflowInstance& instance(const std::string& instanceId);
    std::vector<WorkflowComponent*> availableWork(const std::string& instanceId);
    std::string statusReport(const std::string& instanceId);
    std::string historyReport(const std::string& instanceId);

private:
    struct Entry {
        std::shared_ptr<WorkflowInstance> instance;
        std::shared_ptr<Invoker> invoker;
        std::shared_ptr<NotificationObserver> notifier;
    };
    Entry& entry(const std::string& instanceId);
    const Entry& entry(const std::string& instanceId) const;

    std::map<std::string, std::shared_ptr<const WorkflowDefinition> > definitions_;
    std::map<std::string, Entry> entries_;
    NotificationService* notifier_;
    int nextId_;
};

} // namespace tividy

#endif
