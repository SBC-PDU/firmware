#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>


class InterfaceUtils {
    public:
        static std::string macToString(uint8_t buffer[6], const std::string &separator);
};
