/**
 * Copyright 2022-2023 Roman Ondráček
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This code is based on CC0 licensed example from https://github.com/espressif/esp-idf/blob/release/v5.1/examples/protocols/sntp/main/sntp_example_main.c
 */
#include "network/sntp.h"

Mcp7940n *Ntp::rtc = nullptr;

std::map<std::string, std::string> Ntp::timezones = {
	{"ACST", "ACST-9:30"},
	{"ACST/ACDT", "ACST-9:30ACDT-10:30,M10.5.0/02:00:00,M3.5.0/03:00:00"},
	{"AEST", "AEST-10"},
	{"AEST/AEDT", "AEST-10AEDT-11,M10.1.0/02:00:00,M3.5.0/03:00:00"},
	{"AKST/AKDT", "AKST+9AKDT+8,M3.2.0,M11.1.0"},
	{"AST/ADT", "AST+4ADT+3,M4.1.0/00:01:00,M10.5.0/00:01:00"},
	{"AWST/AWDT", "AWST-8AWDT-9,M12.1.0,M3.5.0/03:00:00"},
	{"BRST/BRDT", "BRST+3BRDT+2,M10.3.0,M2.3.0"},
	{"CET/CEST", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"},
	{"CST", "CST+6"},
	{"CST/CDT", "CST+6CDT+5,M3.2.0,M11.1.0"},
	{"EET/EEST", "EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00"},
	{"EST/EDT", "EST+5EDT+4,M3.2.0,M11.1.0"},
	{"GMT/BST", "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00"},
	{"GMT/IST", "GMT+0IST-1,M3.5.0/01:00:00,M10.5.0/02:00:00"},
	{"HAW", "HAW+10"},
	{"HKT", "HKT-8"},
	{"MSK/MSD", "MSK-3MSD-4,M3.5.0/2,M10.5.0/3"},
	{"RMST/RMDT", "RMST-3RMDT-4,M3.5.0/2,M10.5.0/3"},
	{"MST", "MST+7"},
	{"MST/MDT", "MST+7MDT+6,M3.2.0,M11.1.0"},
	{"NST/NDT", "NST+3:30NDT+2:30,M3.2.0/00:01:00,M11.1.0/00:01:00"},
	{"NZST/NZDT", "NZST-12NZDT-13,M10.1.0/02:00:00,M3.3.0/03:00:00"},
	{"PST/PDT", "PST+8PDT+7,M3.2.0,M11.1.0"},
	{"SGT", "SGT-8"},
	{"ULAT/ULAST", "ULAT-8ULAST-9,M3.5.0/2,M9.5.0/2"},
	{"WET/WEST", "WET-0WEST-1,M3.5.0/01:00:00,M10.5.0/02:00:00"},
	{"WIB", "WIB-7"},
};

void Ntp::notifySyncronization(struct timeval *tv) {
	ESP_LOGI(TAG, "Notification of a time synchronization event");
	time_t epochTime = tv->tv_sec;
	struct tm *timeinfo = localtime(&epochTime);
	Ntp::rtc->setTime(timeinfo);
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

Ntp::Ntp(Mcp7940n *rtc) {
	Ntp::rtc = rtc;
	time_t now;
	struct tm timeinfo;
	char buffer[64];
	time(&now);
	localtime_r(&now, &timeinfo);

	ESP_LOGI(TAG, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);

	// Set timezone to CET/CEST and print local time
	std::string convertedTimezone;
	this->nvs.getString("timezone", convertedTimezone);
	auto timezone = Ntp::timezones.find(convertedTimezone);
	if (timezone != Ntp::timezones.end()) {
		this->timezone = timezone->second;
	} else {
		this->timezone = "UTC";
	}
	ESP_LOGI(TAG, "Timezone: %s - %s", convertedTimezone.c_str(), this->timezone.c_str());
	setenv("TZ", this->timezone.c_str(), 1);
	tzset();
	localtime_r(&now, &timeinfo);
	strftime(buffer, sizeof(buffer), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current date/time is: %s", buffer);

	uint8_t serverCount = 2;
	this->nvs.get("servers", serverCount);
	ESP_LOGI(TAG, "Found %d servers", serverCount);
	for (uint8_t i = 0; i < serverCount; ++i) {
		std::string server;
		std::string key = "server";
		key.push_back('0' + i);
		if(this->nvs.getString(key, server) != ESP_OK) {
			continue;
		}
		this->servers.push_back(server);
		sntp_setservername(i, this->servers[i].c_str());
		ESP_LOGI(TAG, "Server #%u: %s", i, this->servers[i].c_str());
	}
	sntp_set_time_sync_notification_cb(Ntp::notifySyncronization);
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
	sntp_init();

	if (rtc != nullptr && timeinfo.tm_year < (2023 - 1900)) {
		ESP_LOGI(TAG, "Time is not set yet. Getting time from RTC.");
		rtc->getTime(&timeinfo);
		time_t timeSinceEpoch = mktime(&timeinfo);
		struct timeval tv = {
			.tv_sec = timeSinceEpoch,
			.tv_usec = 0,
		};
	    settimeofday(&tv, nullptr);
		time(&now);
	}
	if (timeinfo.tm_year < (2023 - 1900)) {
		ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		Ntp::obtainTime();
		time(&now);
	}

	localtime_r(&now, &timeinfo);
	strftime(buffer, sizeof(buffer), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current date/time is: %s", buffer);

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
