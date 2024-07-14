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
#include "restApi/hostnameController.h"

using namespace sbc_pdu::restApi;

HostnameController::HostnameController(HostnameManager *manager) {
	this->getHandler = {
		.uri = "/api/v1/hostname",
		.method = HTTP_GET,
		.handler = &HostnameController::get,
		.user_ctx = manager,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->putHandler = {
		.uri = "/api/v1/hostname",
		.method = HTTP_PUT,
		.handler = &HostnameController::put,
		.user_ctx = manager,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void HostnameController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getHandler);
	httpd_register_uri_handler(server, &this->putHandler);
}

esp_err_t HostnameController::get(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	HostnameManager *manager = reinterpret_cast<HostnameManager*>(request->user_ctx);
	httpd_resp_set_type(request, "application/json");
	cJSON *root = cJSON_CreateObject();
	std::string hostname = manager->get();
	cJSON_AddStringToObject(root, "hostname", hostname.c_str());
	const char *response = cJSON_PrintUnformatted(root);
	httpd_resp_sendstr(request, response);
	delete response;
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t HostnameController::put(httpd_req_t *request) {
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
	cJSON *hostname = cJSON_GetObjectItem(root, "hostname");
	if (hostname == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"hostname\" property.");
	}
	if (!cJSON_IsString(hostname)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"hostname\" is not a string.");
	}
	if (valid) {
		HostnameManager *manager = reinterpret_cast<HostnameManager*>(request->user_ctx);
		manager->set(std::string(hostname->valuestring));
	}
	httpd_resp_sendstr(request, nullptr);
	cJSON_Delete(root);
	return ESP_OK;
}
