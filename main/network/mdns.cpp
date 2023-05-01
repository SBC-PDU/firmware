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
 */
#include "network/mdns.h"

MulticastDns::MulticastDns(HostnameManager *hostnameManager): hostnameManager(hostnameManager) {
	esp_err_t result = mdns_init();
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "Init failed: %d - %s", result, esp_err_to_name(result));
	} else {
		mdns_hostname_set(this->hostnameManager->get().c_str());
		mdns_instance_name_set("SBC-PDU unit");
		mdns_service_add("HTTP server", "_http", "_tcp", 80, NULL, 0);
	}
	netbiosns_init();
	netbiosns_set_name("sbc-pdu");
}

