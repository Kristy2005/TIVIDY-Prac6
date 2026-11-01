#include "doctest.h"

#include <iostream>
#include <sstream>
#include <string>

void runHospitalAdmissionScenario();
void runUniversityComplaintScenario();

namespace {

struct StandardStreamRedirect {
    std::streambuf* oldInput;
    std::streambuf* oldOutput;
    std::ios::iostate oldInputState;
    std::ios::iostate oldOutputState;

    StandardStreamRedirect(std::istream& input, std::ostream& output)
        : oldInput(std::cin.rdbuf(input.rdbuf())),
          oldOutput(std::cout.rdbuf(output.rdbuf())),
          oldInputState(std::cin.rdstate()),
          oldOutputState(std::cout.rdstate()) {
        std::cin.clear();
        std::cout.clear();
    }

    ~StandardStreamRedirect() {
        std::cin.rdbuf(oldInput);
        std::cout.rdbuf(oldOutput);
        std::cin.clear(oldInputState);
        std::cout.clear(oldOutputState);
    }
};

std::string runWithDecision(void (*scenario)(), char decision) {
    std::istringstream input(std::string(1, decision) + "\n");
    std::ostringstream output;
    StandardStreamRedirect redirect(input, output);
    scenario();
    return output.str();
}

} // namespace

TEST_CASE("Hospital automatic mode preserves the normal admission path") {
    const std::string output = runWithDecision(runHospitalAdmissionScenario, 'n');
    CHECK(output.find("Register patient") != std::string::npos);
    CHECK(output.find("Approve admission") != std::string::npos);
    CHECK(output.find("Emergency clinical review") == std::string::npos);
    CHECK(output.find("Patient admission workflow resolved successfully") != std::string::npos);
    CHECK(output.find("LIVE WORKFLOW CONTROLS") == std::string::npos);
}

TEST_CASE("Hospital automatic mode preserves the urgent admission path") {
    const std::string output = runWithDecision(runHospitalAdmissionScenario, 'y');
    CHECK(output.find("Emergency clinical review") != std::string::npos);
    CHECK(output.find("Patient admission workflow resolved successfully") != std::string::npos);
    CHECK(output.find("LIVE WORKFLOW CONTROLS") == std::string::npos);
}

TEST_CASE("University automatic mode preserves the normal complaint path") {
    const std::string output = runWithDecision(runUniversityComplaintScenario, 'n');
    CHECK(output.find("Record student complaint") != std::string::npos);
    CHECK(output.find("Close complaint case") != std::string::npos);
    CHECK(output.find("Escalation request passed through") == std::string::npos);
    CHECK(output.find("Complaint workflow resolved successfully") != std::string::npos);
    CHECK(output.find("LIVE WORKFLOW CONTROLS") == std::string::npos);
}

TEST_CASE("University automatic mode preserves the senior-review escalation path") {
    const std::string output = runWithDecision(runUniversityComplaintScenario, 'y');
    CHECK(output.find("Escalation request passed through ApprovalHandler") != std::string::npos);
    CHECK(output.find("FinalDecisionHandler") != std::string::npos);
    CHECK(output.find("Complaint workflow resolved successfully") != std::string::npos);
    CHECK(output.find("LIVE WORKFLOW CONTROLS") == std::string::npos);
}
