#pragma once

#include "nvs_manager.h"

/**
 * Hostname manager
 */
class HostnameManager {
    public:
        /**
         * Returns the hostname
         * @return Hostname
         */
        std::string get();
        
        /**
         * Sets the hostname
         * @param hostname Hostname to set
         */
        void set(const std::string &hostname);
    private:
        /// NVS manager
        NvsManager nvs = NvsManager("hostname");
};
