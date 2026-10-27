#include <iostream>
#include <limits>
#include <stdexcept>

#include "engine/Console.h"

void runHospitalAdmissionScenario();
void runUniversityComplaintScenario();

int main() {
    using namespace tividy::console;

    while (true) {
        std::cout << BOLD << "\n========================================\n";
        std::cout << "  TIVIDY WORKFLOW ENGINE - MOCK TUI\n";
        std::cout << "  (progress demo: hard-coded scenarios)\n";
        std::cout << "========================================\n" << RESET;
        std::cout << CYAN    << "1. Hospital admission workflow\n" << RESET;
        std::cout << MAGENTA << "2. University complaint workflow\n" << RESET;
        std::cout << "0. Exit\n";
        std::cout << "Select a scenario: ";

        int choice;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) return 0;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << RED << "Please enter 0, 1 or 2.\n" << RESET;
            continue;
        }

        try {
            if (choice == 1) {
                runHospitalAdmissionScenario();
            } else if (choice == 2) {
                runUniversityComplaintScenario();
            } else if (choice == 0) {
                std::cout << "Exiting TIVIDY demo.\n";
                return 0;
            } else {
                std::cout << RED << "Please select 0, 1 or 2.\n" << RESET;
            }
        } catch (const std::exception& error) {
            std::cerr << RED << "Scenario error: " << error.what() << RESET << '\n';
        } catch (...) {
            std::cerr << RED << "Scenario error: an unexpected internal error occurred." << RESET << '\n';
        }
    }
}