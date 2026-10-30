#ifndef TIVIDY_COMMAND_H
#define TIVIDY_COMMAND_H
#include "WorkflowMemento.h"
namespace tividy {
class WorkflowInstance;

/**
 * @brief Abstract base class for workflow commands.
 *
 * A command keeps a non-owning pointer to the workflow instance it acts on and
 * owns the most recently saved memento. Concrete commands implement execute()
 * and undo(). The memento implementation currently stores a raw state pointer;
 * see WorkflowMemento and WorkflowInstance::createMemento() for the associated
 * lifetime limitation.
 *
 */

class Command {
protected:
    /** @brief Receiver of the command; not owned by the command. */
    WorkflowInstance* receiver;

    /** @brief Most recently created snapshot; deleted by this command. */
    WorkflowMemento* previous;

    /**
     * @brief Replace the stored snapshot with one made from the receiver.
     *
     * Does nothing when receiver is null. Any previously stored memento is
     * deleted before a replacement is requested.
     */
    void saveState();
public:
    /** @brief Construct a command without a receiver or saved memento. */
    Command();

    /**
     * @brief Construct a command for a workflow instance.
     * @param instance Receiver to retain without taking ownership; may be null.
     */
    explicit Command(WorkflowInstance* instance);

    /** @brief Delete the memento currently retained by the command. */
    virtual ~Command();

    /**
     * @brief Change the workflow instance that receives this command.
     * @param instance Receiver to retain without taking ownership; may be null.
     */
    void setReceiver(WorkflowInstance* instance);
    
    /**
     * @brief Execute the command's operation.
     * @throws std::exception Concrete commands may report invalid setup or
     *         rejected workflow transitions by throwing an exception.
     */
    virtual void execute() = 0;
    
    /**
     * @brief Undo the command using the state saved during execution.
     * @throws std::exception Concrete commands may throw when no valid undo
     *         snapshot is available or the restore operation fails.
     */
    virtual void undo() = 0;
};
}
#endif
