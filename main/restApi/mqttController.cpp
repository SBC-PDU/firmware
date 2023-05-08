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
#include "restApi/mqttController.h"

using namespace sbc_pdu::restApi;

MqttController::MqttController() {
	this->getHandler = {
		.uri = "/api/v1/mqtt",
		.method = HTTP_GET,
		.handler = &MqttController::get,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->putHandler = {
		.uri = "/api/v1/mqtt",
		.method = HTTP_PUT,
		.handler = &MqttController::put,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void MqttController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getHandler);
	httpd_register_uri_handler(server, &this->putHandler);
}

esp_err_t MqttController::get(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	NvsManager nvs = NvsManager("mqtt");
	httpd_resp_set_type(request, "application/json");
	cJSON *root = cJSON_CreateObject();
	std::string uri;
	if (nvs.getString("uri", uri) != ESP_OK) {
		uri = "";
	}
	cJSON_AddStringToObject(root, "uri", uri.c_str());
	std::string username;
	if (nvs.getString("username", username) != ESP_OK) {
		username = "";
	}
	cJSON_AddStringToObject(root, "username", username.c_str());
	std::string password;
	if (nvs.getString("password", password) != ESP_OK) {
		password = "";
	}
	cJSON_AddStringToObject(root, "password", password.c_str());
	const char *response = cJSON_PrintUnformatted(root);
	httpd_resp_sendstr(request, response);
	delete response;
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t MqttController::put(httpd_req_t *request) {
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
	NvsManager nvs = NvsManager("mqtt");
	cJSON *uri = cJSON_GetObjectItem(root, "uri");
	if (uri == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"uri\" property.");
	}
	if (!cJSON_IsString(uri)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"uri\" is not a string.");
	}
	cJSON *username = cJSON_GetObjectItem(root, "username");
	if (username == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"username\" property.");
	}
	if (!cJSON_IsString(username)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"username\" is not a string.");
	}
	cJSON *password = cJSON_GetObjectItem(root, "password");
	if (password == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"password\" property.");
	}
	if (!cJSON_IsString(password)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"password\" is not a string.");
	}
	if (valid) {
		nvs.setString("uri", std::string(uri->valuestring));
		nvs.setString("username", std::string(username->valuestring));
		nvs.setString("password", std::string(password->valuestring));
		nvs.commit();
	}
	httpd_resp_sendstr(request, nullptr);
	cJSON_Delete(root);
	return ESP_OK;
}
