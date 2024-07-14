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
#include "restApi/authController.h"

using namespace sbc_pdu::restApi;

AuthController::AuthController() {
	this->getHandler = {
		.uri = "/api/v1/auth",
		.method = HTTP_GET,
		.handler = &AuthController::get,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->putHandler = {
		.uri = "/api/v1/auth",
		.method = HTTP_PUT,
		.handler = &AuthController::put,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void AuthController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getHandler);
	httpd_register_uri_handler(server, &this->putHandler);
}

esp_err_t AuthController::get(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	httpd_resp_set_type(request, "text/plain");
	httpd_resp_sendstr(request, "OK");
	return ESP_OK;
}

esp_err_t AuthController::put(httpd_req_t *request) {
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
	NvsManager nvs = NvsManager("httpCredentials");
	cJSON *username = cJSON_GetObjectItem(root, "username");
	if (username == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"username\" property.");
	}
	if (!cJSON_IsString(username)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"username\" is not a string.");
	}
	cJSON *oldPassword = cJSON_GetObjectItem(root, "oldPassword");
	if (oldPassword == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"oldPassword\" property.");
	}
	if (!cJSON_IsString(oldPassword)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"oldPassword\" is not a string.");
	}
	std::string oldPasswordString = std::string(oldPassword->valuestring);
	std::string currentPassword;
	nvs.getString("password", currentPassword);
	if (oldPasswordString != currentPassword) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Incorrect current password.");
	}
	cJSON *newPassword = cJSON_GetObjectItem(root, "newPassword");
	if (newPassword == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"newPassword\" property.");
	}
	if (!cJSON_IsString(newPassword)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"newPassword\" is not a string.");
	}
	if (valid) {
		nvs.setString("username", std::string(username->valuestring));
		nvs.setString("password", std::string(newPassword->valuestring));
		nvs.commit();
	}
	httpd_resp_sendstr(request, nullptr);
	cJSON_Delete(root);
	return ESP_OK;
}
