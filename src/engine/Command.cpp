#include "Command.h"
#include "WorkflowInstance.h"
namespace tividy {
Command::Command() : receiver(nullptr), previous(nullptr) {}
Command::Command(WorkflowInstance* instance) : receiver(instance), previous(nullptr) {}
Command::~Command() { delete previous; }
void Command::setReceiver(WorkflowInstance* instance) { receiver = instance; }
void Command::saveState() { if (receiver != nullptr) { delete previous; previous = receiver->createMemento(); } }
}
