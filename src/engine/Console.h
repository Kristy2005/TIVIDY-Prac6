#ifndef TIVIDY_CONSOLE_H
#define TIVIDY_CONSOLE_H

#include <iostream>
#include <string>

/**
 * @file Console.h
 * @brief ANSI colour codes for TUI output readability.
 *
 * Works in most Linux and macOS terminals, and in modern Windows terminals
 * (Windows Terminal, VS Code's integrated terminal). If raw escape codes
 * show up as text instead of colour, the terminal does not support ANSI
 * escape sequences.
 *
 * Every coloured line must end with RESET, or the colour bleeds into
 * whatever is printed next (including plain cout lines elsewhere in the
 * engine).
 */

namespace tividy {
namespace console {

const char* const RESET   = "\033[0m";
const char* const BOLD    = "\033[1m";

const char* const CYAN    = "\033[36m";  ///< Hospital admission scenario
const char* const MAGENTA = "\033[35m";  ///< University complaint scenario
const char* const GREEN   = "\033[32m";  ///< Completed work, successful result
const char* const YELLOW  = "\033[33m";  ///< Work in progress
const char* const RED     = "\033[31m";  ///< Escalation / exception path
const char* const BLUE    = "\033[34m";  ///< Engine / observer chatter

} // namespace console
} // namespace tividy

#endif // TIVIDY_CONSOLE_H