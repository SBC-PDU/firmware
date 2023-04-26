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
#include "restApi/systemController.h"

using namespace sbc_pdu::restApi;

SystemController::SystemController() {
	this->getInfoHandler = {
		.uri = "/api/v1/system/info",
		.method = HTTP_GET,
		.handler = &SystemController::getInfo,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->restartHandler = {
		.uri = "/api/v1/system/restart",
		.method = HTTP_POST,
		.handler = &SystemController::restart,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void SystemController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getInfoHandler);
	httpd_register_uri_handler(server, &this->restartHandler);
}

void SystemController::setChipInfo(cJSON *root) {
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	cJSON *chipInfo = cJSON_AddObjectToObject(root, "chip");
	cJSON_AddNumberToObject(chipInfo, "cores", chip_info.cores);
	std::map<esp_chip_model_t, std::string> modelMap = {
		{CHIP_ESP32, "ESP32"},
		{CHIP_ESP32S2, "ESP32-S2"},
		{CHIP_ESP32S3, "ESP32-S3"},
		{CHIP_ESP32C3, "ESP32-C3"},
		{CHIP_ESP32H2, "ESP32-H2"},
		{CHIP_ESP32C2, "ESP32-C2"},
	};
	auto foundModel = modelMap.find(chip_info.model);
	std::string model = foundModel != modelMap.end() ? foundModel->second : "UNKNOWN";
	cJSON_AddStringToObject(chipInfo, "model", model.c_str());
	cJSON_AddNumberToObject(chipInfo, "revision", chip_info.revision);
}

void SystemController::setNetworkInfo(cJSON *root) {
	cJSON *networkInterfaces = cJSON_AddArrayToObject(root, "network");
	esp_netif_t *netif = nullptr;
	for (size_t i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
		netif = esp_netif_next(netif);
		cJSON *networkInterface = cJSON_CreateObject();
		cJSON_AddStringToObject(networkInterface, "name", esp_netif_get_desc(netif));
		uint8_t mac[6] = {0,};
		if (esp_netif_get_mac(netif, mac) == ESP_OK) {
			cJSON_AddStringToObject(networkInterface, "macAddress", InterfaceUtils::macToString(mac, ":").c_str());
		}
		cJSON *ipv4 = cJSON_AddObjectToObject(networkInterface, "ipv4");
		esp_netif_ip_info_t ip;
		if (esp_netif_get_ip_info(netif, &ip) == ESP_OK) {
			char address[40] = "";
			snprintf(address, sizeof(address), IPSTR, IP2STR(&ip.ip));
			cJSON_AddStringToObject(ipv4, "address", address);
			snprintf(address, sizeof(address), IPSTR, IP2STR(&ip.netmask));
			cJSON_AddStringToObject(ipv4, "netmask", address);
			snprintf(address, sizeof(address), IPSTR, IP2STR(&ip.gw));
			cJSON_AddStringToObject(ipv4, "gateway", address);
		}
#if CONFIG_LWIP_IPV6
		cJSON *ipv6 = cJSON_AddObjectToObject(networkInterface, "ipv6");
		cJSON *addresses = cJSON_AddArrayToObject(ipv6, "addresses");
		esp_ip6_addr_t ips[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
		int ipCount = esp_netif_get_all_ip6(netif, ips);
		for (int j = 0; j< ipCount; ++j) {
			char address[40] = "";
			snprintf(address, sizeof(address), IPV6STR, IPV62STR(ips[j]));
			cJSON_AddItemToArray(addresses, cJSON_CreateString(address));
		}
#endif
		esp_netif_dns_info_t dnsInfo;
		cJSON *dns = cJSON_AddArrayToObject(networkInterface, "dns");
		for (int j = ESP_NETIF_DNS_MAIN; j < ESP_NETIF_DNS_MAX; ++j) {
			if (esp_netif_get_dns_info(netif, static_cast<esp_netif_dns_type_t>(j), &dnsInfo) == ESP_OK) {
				char address[40] = "";
				if (dnsInfo.ip.type == ESP_IPADDR_TYPE_V4) {
					snprintf(address, sizeof(address), IPSTR, IP2STR(&dnsInfo.ip.u_addr.ip4));
				} else if (dnsInfo.ip.type == ESP_IPADDR_TYPE_V6) {
					snprintf(address, sizeof(address), IPV6STR, IPV62STR(dnsInfo.ip.u_addr.ip6));
				} else {
					continue;
				}
				if (strcmp(address, "0.0.0.0") == 0) {
					continue;
				}
				cJSON_AddItemToArray(dns, cJSON_CreateString(address));
			}
		}
		const char *hostname;
		if (esp_netif_get_hostname(netif, &hostname) == ESP_OK) {
			cJSON_AddStringToObject(networkInterface, "hostname", hostname);
		}
		cJSON_AddBoolToObject(networkInterface, "isUp", esp_netif_is_netif_up(netif));
		cJSON_AddItemToArray(networkInterfaces, networkInterface);
	}
}

esp_err_t SystemController::getInfo(httpd_req_t *request) {
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	httpd_resp_set_type(request, "application/json");
	cJSON *root = cJSON_CreateObject();
	SystemController::setChipInfo(root);
	SystemController::setNetworkInfo(root);

	// Heap
	cJSON *heap = cJSON_AddObjectToObject(root, "heap");
	cJSON_AddNumberToObject(heap, "total", heap_caps_get_total_size(MALLOC_CAP_8BIT));
	cJSON_AddNumberToObject(heap, "free", heap_caps_get_free_size(MALLOC_CAP_8BIT));

	cJSON_AddStringToObject(root, "idfVersion", IDF_VER);
	uint64_t uptime = esp_timer_get_time() / 1000000.0;
	cJSON_AddNumberToObject(root, "uptime", uptime);
	const esp_app_desc_t *appDescription = esp_app_get_description();
	cJSON_AddStringToObject(root, "version", appDescription->version);
	const char *json = cJSON_PrintUnformatted(root);
	httpd_resp_sendstr(request, json);
	delete json;
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t SystemController::restart(httpd_req_t *request) {
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	httpd_resp_sendstr(request, "");
	esp_restart();
	return ESP_OK;
}
