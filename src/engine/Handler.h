/**
 * @file Handler.h
 * @brief Chain of Responsibility: escalation passes along handlers until one can deal with it.
 */
#ifndef TIVIDY_ENGINE_HANDLER_H
#define TIVIDY_ENGINE_HANDLER_H

#include <memory>
#include <string>
#include <vector>
#include "engine/Participant.h"

namespace tividy {

/** @brief The request that travels along the chain. */
struct EscalationRequest {
    std::string instanceId;
    std::string reason;
    int requiredLevel;                      ///< Authority needed (grows with each escalation).
    std::vector<Participant*> participants; ///< Everyone in the instance.
    Participant* handledBy;                 ///< Filled in by the handler that accepts.
    std::vector<std::string> trail;         ///< Who looked at it, in order (for the audit log).

    EscalationRequest() : requiredLevel(1), handledBy(0) {}
};

/**
 * @brief One link of the escalation chain. A handler accepts a request when its
 * authority is high enough AND a participant with its role is available;
 * otherwise it passes the request on. Each link owns the next one.
 */
class Handler {
public:
    Handler(const std::string& role, int authority);
    virtual ~Handler();

    /** @brief Append @p next to the end of this chain (takes ownership). @return @p next. */
    Handler* append(Handler* next);
    /** @brief Offer the request to this link, then the rest of the chain. @return true if someone accepted. */
    bool handle(EscalationRequest& request) const;

    const std::string& role() const { return role_; }
    int authority() const { return authority_; }
    const Handler* next() const { return next_.get(); }
    /** @return Label for the trail, e.g. "Work". */
    virtual std::string kind() const = 0;

protected:
    /** @brief Does this link have enough authority? Subclasses may change the rule. */
    virtual bool canHandle(const EscalationRequest& request) const;

private:
    Handler(const Handler&);
    Handler& operator=(const Handler&);

    std::string role_;
    int authority_;
    std::unique_ptr<Handler> next_;
};

/** @brief First line: ordinary work-level responsibility. */
class WorkHandler : public Handler {
public:
    WorkHandler(const std::string& role, int authority) : Handler(role, authority) {}
    std::string kind() const { return "Work"; }
};

/** @brief Middle link: can approve or reject on behalf of the process. */
class ApprovalHandler : public Handler {
public:
    ApprovalHandler(const std::string& role, int authority) : Handler(role, authority) {}
    std::string kind() const { return "Approval"; }
};

/** @brief Senior link: takes over cases the lower levels cannot resolve. */
class EscalationHandler : public Handler {
public:
    EscalationHandler(const std::string& role, int authority) : Handler(role, authority) {}
    std::string kind() const { return "Escalation"; }
};

/** @brief Last link: accepts any request regardless of level (if somebody holds its role). */
class FinalDecisionHandler : public Handler {
public:
    explicit FinalDecisionHandler(const std::string& role) : Handler(role, 1000000) {}
    std::string kind() const { return "FinalDecision"; }

protected:
    bool canHandle(const EscalationRequest&) const { return true; }
};

} // namespace tividy

#endif
