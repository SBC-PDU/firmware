#pragma once

#include <exception>
#include <string>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_vfs.h>

class SPIFFS {
    public:
        explicit SPIFFS(const std::string &basePath);
};