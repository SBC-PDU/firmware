#pragma once

#include <cstring>
#include <ctime>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_sntp.h>

#include "nvs_manager.h"

/**
 * Simple Network Time Protocol client
 */
class Ntp {
    public:
        /**
         * @brief Construct a new SNTP client
         */
        Ntp();
        
        /**
         * @brief Obtains the current time from NTP servers
         */
        static void obtainTime();

        /**
         * @brief Notify time syncchronization
         * 
         * @param tv Time
         */
        static void notifySyncronization(struct timeval *tv);

    private:
        /// @brief NVS manager
        NvsManager nvs = NvsManager("ntp");
        /// @brief  NTP server list
        std::vector<std::string> servers;
        /// @brief Timezone
        std::string timezone;
};
