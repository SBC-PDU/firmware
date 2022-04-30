#include "interfaceUtils.h"

std::string InterfaceUtils::macToString(uint8_t buffer[6], const std::string &separator) {
    std::stringstream address;
    for (uint8_t i = 0; i < 6; ++i) {
        address << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        if (i != 5) {
            address << separator;
        }
    }
    return address.str();
}