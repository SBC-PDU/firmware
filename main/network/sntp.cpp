#include "network/sntp.h"

static const char *TAG = "NTP";

void Ntp::notifySyncronization(struct timeval *tv) {
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void Ntp::obtainTime() {
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t maxRetries = 10;
    for (uint8_t retry = 0; sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < maxRetries; ++retry) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, maxRetries);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

Ntp::Ntp() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    for (uint8_t i = 0; i < 2; ++i) {
        std::string server;
        std::string key = "server";
        key.push_back('0' + i);
        this->nvs.getString(key, server);
        this->servers.push_back(server);
        sntp_setservername(i, this->servers[i].c_str());
    }
    sntp_set_time_sync_notification_cb(Ntp::notifySyncronization);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2021 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        Ntp::obtainTime();
        time(&now);
    }

    char strftime_buf[64];

    // Set timezone to CET/CEST and print local time
    this->nvs.getString("timezone", this->timezone);
    setenv("TZ", this->timezone.c_str(), 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
            adjtime(NULL, &outdelta);
            ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                        (long)outdelta.tv_sec,
                        outdelta.tv_usec/1000,
                        outdelta.tv_usec%1000);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}
