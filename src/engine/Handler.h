#ifndef HANDLER_H
#define HANDLER_H

namespace tividy {

class WorkflowInstance;
/**
 * @brief Base class for links in a Chain of Responsibility.
 *
 * Each handler retains an optional pointer to the next handler. The successor
 * is non-owning and must remain alive while it can be called. The base
 * implementation validates a request and forwards it when a successor exists;
 * with no successor it returns without further action.
 *
 */

class Handler {
protected:
    /** @brief Next handler in the chain; not owned. */
    Handler* successor;

public:
    /** @brief Construct a handler with no successor. */
    Handler();

    /** @brief Virtual destructor for safe deletion through a Handler pointer. */
    virtual ~Handler();

    /**
     * @brief Set the next handler in the chain.
     * @param handler Next handler to call, or null to end the chain here; not owned.
     */
    virtual void setSuccessor(Handler* handler);
    
    /**
     * @brief Validate and optionally forward a workflow request.
     * @param instance Workflow instance to forward; must not be null.
     * @throws std::invalid_argument If @p instance is null.
     */
    virtual void handleRequest(WorkflowInstance* instance);
};

}

#endif