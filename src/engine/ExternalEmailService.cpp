#include "ExternalEmailService.h"
#include <iostream>
namespace tividy {
void ExternalEmailService::sendEmail(std::string address, std::string body) {
    std::cout << "Email to " << address << ": " << body << std::endl;
}
}
