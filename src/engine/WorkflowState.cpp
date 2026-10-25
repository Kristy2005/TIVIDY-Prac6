#include "engine/WorkflowState.h"
#include "engine/WorkflowInstance.h"

namespace tividy {

WorkflowState::~WorkflowState() {}

void WorkflowState::reject(const WorkflowInstance& w, const char* action) const {
    throw InvalidTransition(std::string("Cannot ") + action + " while workflow " + w.id() + " is " + name());
}

// Default behaviour: not allowed in this state.
void WorkflowState::start(WorkflowInstance& w, const std::string&) { reject(w, "start"); }
void WorkflowState::assign(WorkflowInstance& w, const std::string&, const std::string&, const std::string&) { reject(w, "assign work"); }
void WorkflowState::complete(WorkflowInstance& w, const std::string&, const std::string&, const std::string&) { reject(w, "complete work"); }
void WorkflowState::cancelItem(WorkflowInstance& w, const std::string&, const std::string&, const std::string&) { reject(w, "cancel work"); }
void WorkflowState::escalate(WorkflowInstance& w, const std::string&, const std::string&) { reject(w, "escalate"); }
void WorkflowState::resolveEscalation(WorkflowInstance& w, const std::string&, const std::string&) { reject(w, "resolve an escalation"); }
void WorkflowState::close(WorkflowInstance& w, const std::string&, const std::string&) { reject(w, "close"); }
void WorkflowState::onWorkAssigned(WorkflowInstance&) {}
void WorkflowState::onAllWorkFinished(WorkflowInstance&) {}

// Forwarders (this class is a friend of WorkflowInstance; its subclasses are not).
void WorkflowState::performStart(WorkflowInstance& w, const std::string& a) { w.doStart(a); }
void WorkflowState::performAssign(WorkflowInstance& w, const std::string& i, const std::string& p, const std::string& a) { w.doAssign(i, p, a); }
void WorkflowState::performComplete(WorkflowInstance& w, const std::string& i, const std::string& a, const std::string& o) { w.doComplete(i, a, o); }
void WorkflowState::performCancelItem(WorkflowInstance& w, const std::string& i, const std::string& a, const std::string& r) { w.doCancelItem(i, a, r); }
void WorkflowState::performEscalate(WorkflowInstance& w, const std::string& a, const std::string& r) { w.doEscalate(a, r); }
void WorkflowState::performResolveEscalation(WorkflowInstance& w, const std::string& a, const std::string& n) { w.doResolveEscalation(a, n); }
void WorkflowState::performClose(WorkflowInstance& w, const std::string& a, const std::string& r) { w.doClose(a, r); }
void WorkflowState::transition(WorkflowInstance& w, WorkflowState& next, const std::string& a, const std::string& r) { w.transitionTo(next, a, r); }

WorkflowState& WorkflowState::byName(const std::string& n) {
    if (n == "Submitted")  return SubmittedState::instance();
    if (n == "Active")     return ActiveState::instance();
    if (n == "InProgress") return InProgressState::instance();
    if (n == "Escalated")  return EscalatedState::instance();
    if (n == "Resolved")   return ResolvedState::instance();
    if (n == "Closed")     return ClosedState::instance();
    throw WorkflowError("Unknown workflow state '" + n + "'");
}

#define TIVIDY_DEFINE_STATE(Class, Label)                 \
    Class& Class::instance() { static Class s; return s; } \
    std::string Class::name() const { return Label; }

TIVIDY_DEFINE_STATE(SubmittedState, "Submitted")
TIVIDY_DEFINE_STATE(ActiveState, "Active")
TIVIDY_DEFINE_STATE(InProgressState, "InProgress")
TIVIDY_DEFINE_STATE(EscalatedState, "Escalated")
TIVIDY_DEFINE_STATE(ResolvedState, "Resolved")
TIVIDY_DEFINE_STATE(ClosedState, "Closed")
#undef TIVIDY_DEFINE_STATE

// ---- Submitted ------------------------------------------------------------
void SubmittedState::start(WorkflowInstance& w, const std::string& actor) {
    transition(w, ActiveState::instance(), actor, "started");
    performStart(w, actor);
}
void SubmittedState::close(WorkflowInstance& w, const std::string& actor, const std::string& reason) { performClose(w, actor, reason); }

// ---- Active -----------------------------------------------------------------
void ActiveState::assign(WorkflowInstance& w, const std::string& i, const std::string& p, const std::string& a) { performAssign(w, i, p, a); }
void ActiveState::cancelItem(WorkflowInstance& w, const std::string& i, const std::string& a, const std::string& r) { performCancelItem(w, i, a, r); }
void ActiveState::close(WorkflowInstance& w, const std::string& a, const std::string& r) { performClose(w, a, r); }
void ActiveState::onWorkAssigned(WorkflowInstance& w) { transition(w, InProgressState::instance(), "system", "work assigned"); }
void ActiveState::onAllWorkFinished(WorkflowInstance& w) { transition(w, ResolvedState::instance(), "system", "all work finished"); }

// ---- InProgress ---------------------------------------------------------------
void InProgressState::assign(WorkflowInstance& w, const std::string& i, const std::string& p, const std::string& a) { performAssign(w, i, p, a); }
void InProgressState::complete(WorkflowInstance& w, const std::string& i, const std::string& a, const std::string& o) { performComplete(w, i, a, o); }
void InProgressState::cancelItem(WorkflowInstance& w, const std::string& i, const std::string& a, const std::string& r) { performCancelItem(w, i, a, r); }
void InProgressState::escalate(WorkflowInstance& w, const std::string& a, const std::string& r) { performEscalate(w, a, r); }
void InProgressState::close(WorkflowInstance& w, const std::string& a, const std::string& r) { performClose(w, a, r); }
void InProgressState::onAllWorkFinished(WorkflowInstance& w) { transition(w, ResolvedState::instance(), "system", "all work finished"); }

// ---- Escalated -----------------------------------------------------------------
void EscalatedState::resolveEscalation(WorkflowInstance& w, const std::string& a, const std::string& n) { performResolveEscalation(w, a, n); }
void EscalatedState::close(WorkflowInstance& w, const std::string& a, const std::string& r) { performClose(w, a, r); }

// ---- Resolved ------------------------------------------------------------------
void ResolvedState::close(WorkflowInstance& w, const std::string& a, const std::string& r) { performClose(w, a, r); }

} // namespace tividy
