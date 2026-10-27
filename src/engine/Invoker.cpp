#include "Invoker.h"
#include <stdexcept>
namespace tividy {
Invoker::Invoker() : command(nullptr) {}
void Invoker::setCommand(Command* value) { command = value; }
void Invoker::executeCommand() { if (!command) throw std::logic_error("Invoker: no command set"); command->execute(); }
void Invoker::undoCommand() { if (!command) throw std::logic_error("Invoker: no command set"); command->undo(); }
}
