/**
 * Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>
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
 */
#include "network/hostname.h"
#include "network/wifi.h"

std::string HostnameManager::get() {
	std::string hostname;
	esp_err_t result = this->nvs.getString("hostname", hostname);
	if (result != ESP_OK) {
		hostname = "sbc-pdu_" + Wifi::getPrimaryMacAddress();
		this->set(hostname);
	}
	return hostname;
}

void HostnameManager::set(const std::string &hostname) {
	ESP_ERROR_CHECK(this->nvs.setString("hostname", hostname));
}
