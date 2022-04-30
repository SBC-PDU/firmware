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

class Ntp {
    public:
        Ntp();
        static void obtainTime();
        static void notifySyncronization(struct timeval *tv);

    private:
        /**
         * NVS manager
         */
        NvsManager nvs = NvsManager("ntp");

        std::vector<std::string> servers;
        std::string timezone;
};

void ntp_init(void);