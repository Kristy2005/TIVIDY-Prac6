#include <iostream>
#include <limits>
#include <stdexcept>

void runHospitalAdmissionScenario();
void runUniversityComplaintScenario();

int main() {
    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "       TIVIDY WORKFLOW ENGINE DEMO\n";
        std::cout << "========================================\n";
        std::cout << "1. Hospital admission workflow\n";
        std::cout << "2. University complaint workflow\n";
        std::cout << "0. Exit\n";
        std::cout << "Select a scenario: ";

        int choice;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) return 0;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter 0, 1 or 2.\n";
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
                std::cout << "Please select 0, 1 or 2.\n";
            }
        } catch (const std::exception& error) {
            std::cerr << "Scenario error: " << error.what() << '\n';
        }
    }
}
