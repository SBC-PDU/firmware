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
#include "restApi/ntpController.h"

using namespace sbc_pdu::restApi;

NtpController::NtpController() {
	this->getHandler = {
		.uri = "/api/v1/ntp",
		.method = HTTP_GET,
		.handler = &NtpController::get,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->putHandler = {
		.uri = "/api/v1/ntp",
		.method = HTTP_PUT,
		.handler = &NtpController::put,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void NtpController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getHandler);
	httpd_register_uri_handler(server, &this->putHandler);
}

esp_err_t NtpController::get(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	NvsManager nvs = NvsManager("ntp");
	httpd_resp_set_type(request, "application/json");
	cJSON *root = cJSON_CreateObject();
	std::vector<std::string> serversVector;
	uint8_t serverCount = 2;
	nvs.get("servers", serverCount);
	cJSON *servers = cJSON_AddArrayToObject(root, "servers");
	for (uint8_t i = 0; i < serverCount; ++i) {
		std::string server;
		std::string key = "server";
		key.push_back('0' + i);
		if(nvs.getString(key, server) != ESP_OK) {
			continue;
		}
		serversVector.push_back(server);
		cJSON_AddItemToArray(servers, cJSON_CreateString(serversVector[i].c_str()));
	}
	std::string timezone;
	if (nvs.getString("timezone", timezone) != ESP_OK) {
		timezone = "";
	}
	cJSON_AddStringToObject(root, "timezone", timezone.c_str());
	const char *response = cJSON_PrintUnformatted(root);
	httpd_resp_sendstr(request, response);
	delete response;
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t NtpController::put(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	cJSON *root = nullptr;
	esp_err_t result = RestApiUtils::parseJsonRequest(request, &root);
	if (result != ESP_OK) {
		return result;
	}
	bool valid = true;
	NvsManager nvs = NvsManager("ntp");
	cJSON *servers = cJSON_GetObjectItem(root, "servers");
	if (servers == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"servers\" property.");
	}
	if (!cJSON_IsArray(servers)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"servers\" is not an array.");
	}
	int serversSize = cJSON_GetArraySize(servers);
	if (serversSize == 0) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"servers\" is an empty array.");
	}
	if (serversSize > 2) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Array \"servers\" is longer than 2 entries.");
	}
	uint8_t currentServers = 2;
	nvs.get("servers", currentServers);
	// Validate new servers
	for (int i = 0; i < serversSize; ++i) {
		cJSON *server = cJSON_GetArrayItem(servers, i);
		if (!cJSON_IsString(server)) {
			valid = false;
			RestApiUtils::createBadRequestResponse(request, "Property \"servers\" is not array of strings.");
		}
	}
	cJSON *timezone = cJSON_GetObjectItem(root, "timezone");
	if (timezone == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"timezone\" property.");
	}
	if (!cJSON_IsString(timezone)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"timezone\" is not a string.");
	}
	if (Ntp::timezones.find(std::string(timezone->valuestring)) == Ntp::timezones.end()) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"timezone\" is not a valid timezone.");
	}
	if (valid) {
		// Remove old servers
		for (uint8_t i = serversSize; i < currentServers; ++i) {
			std::string server;
			std::string key = "server";
			key.push_back('0' + i);
			nvs.remove(key);
		}
		// Set new servers
		for (int i = 0; i < serversSize; ++i) {
			cJSON *server = cJSON_GetArrayItem(servers, i);
			std::string key = "server";
			key.push_back('0' + i);
			nvs.setString(key, std::string(server->valuestring));
		}
		nvs.setString("timezone", std::string(timezone->valuestring));
		nvs.commit();
	}
	httpd_resp_sendstr(request, nullptr);
	cJSON_Delete(root);
	return ESP_OK;
}
