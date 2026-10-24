#include "engine/TIVIDYFacade.h"
#include <sstream>

namespace tividy {

TIVIDYFacade::TIVIDYFacade(NotificationService* notifier) : notifier_(notifier), nextId_(1) {}

void TIVIDYFacade::setNotificationService(NotificationService* notifier) { notifier_ = notifier; }

void TIVIDYFacade::registerWorkflow(std::shared_ptr<const WorkflowDefinition> definition) {
    if (!definition) throw WorkflowError("Cannot register a null definition");
    definitions_[definition->name()] = definition;
}

std::vector<std::string> TIVIDYFacade::workflowNames() const {
    std::vector<std::string> names;
    for (std::map<std::string, std::shared_ptr<const WorkflowDefinition> >::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        names.push_back(it->first);
    return names;
}

std::string TIVIDYFacade::createInstance(const std::string& workflowName) {
    std::map<std::string, std::shared_ptr<const WorkflowDefinition> >::const_iterator def = definitions_.find(workflowName);
    if (def == definitions_.end()) throw WorkflowError("Unknown workflow '" + workflowName + "'");

    std::ostringstream id;
    id << "WF-" << nextId_++;
    Entry e;
    e.instance.reset(new WorkflowInstance(id.str(), def->second));
    e.invoker.reset(new Invoker());
    if (notifier_) {
        e.notifier.reset(new NotificationObserver(*notifier_, &e.instance->historyObserver()));
        e.instance->attach(e.notifier.get());
    }
    entries_[id.str()] = e;
    return id.str();
}

TIVIDYFacade::Entry& TIVIDYFacade::entry(const std::string& instanceId) {
    std::map<std::string, Entry>::iterator it = entries_.find(instanceId);
    if (it == entries_.end()) throw WorkflowError("Unknown instance '" + instanceId + "'");
    return it->second;
}

const TIVIDYFacade::Entry& TIVIDYFacade::entry(const std::string& instanceId) const {
    std::map<std::string, Entry>::const_iterator it = entries_.find(instanceId);
    if (it == entries_.end()) throw WorkflowError("Unknown instance '" + instanceId + "'");
    return it->second;
}

std::vector<std::string> TIVIDYFacade::instanceIds() const {
    std::vector<std::string> ids;
    for (std::map<std::string, Entry>::const_iterator it = entries_.begin(); it != entries_.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

void TIVIDYFacade::addParticipant(const std::string& id, const std::string& name, const std::string& role) {
    entry(id).instance->addParticipant(name, role);
}

void TIVIDYFacade::setData(const std::string& id, const std::string& key, const std::string& value) {
    entry(id).instance->setData(key, value);
}

void TIVIDYFacade::startWorkflow(const std::string& id, const std::string& actor) {
    Entry& e = entry(id);
    e.invoker->execute(new StartWorkflowCommand(*e.instance, actor));
}

void TIVIDYFacade::assignWork(const std::string& id, const std::string& itemId, const std::string& participant,
                              const std::string& actor) {
    Entry& e = entry(id);
    e.invoker->execute(new AssignWorkCommand(*e.instance, itemId, participant, actor));
}

void TIVIDYFacade::completeWork(const std::string& id, const std::string& itemId, const std::string& actor,
                                const std::string& outcome) {
    Entry& e = entry(id);
    e.invoker->execute(new CompleteWorkCommand(*e.instance, itemId, actor, outcome));
}

void TIVIDYFacade::escalateWork(const std::string& id, const std::string& actor, const std::string& reason) {
    Entry& e = entry(id);
    e.invoker->execute(new EscalateWorkCommand(*e.instance, actor, reason));
}

void TIVIDYFacade::approveEscalation(const std::string& id, const std::string& actor, const std::string& notes) {
    Entry& e = entry(id);
    e.invoker->execute(new ApproveWorkCommand(*e.instance, actor, notes));
}

void TIVIDYFacade::cancelWork(const std::string& id, const std::string& itemId, const std::string& actor,
                              const std::string& reason) {
    Entry& e = entry(id);
    e.invoker->execute(new CancelWorkCommand(*e.instance, itemId, actor, reason));
}

void TIVIDYFacade::closeWorkflow(const std::string& id, const std::string& actor, const std::string& reason) {
    Entry& e = entry(id);
    e.invoker->execute(new CloseWorkflowCommand(*e.instance, actor, reason));
}

void TIVIDYFacade::undo(const std::string& id) { entry(id).invoker->undoLast(); }
bool TIVIDYFacade::canUndo(const std::string& id) const { return entry(id).invoker->canUndo(); }

WorkflowInstance& TIVIDYFacade::instance(const std::string& id) { return *entry(id).instance; }

std::vector<WorkflowComponent*> TIVIDYFacade::availableWork(const std::string& id) {
    return entry(id).instance->availableWork();
}

std::string TIVIDYFacade::statusReport(const std::string& id) { return entry(id).instance->describe(); }
std::string TIVIDYFacade::historyReport(const std::string& id) { return entry(id).instance->history().report(); }

} // namespace tividy
