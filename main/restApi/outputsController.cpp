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
#include "restApi/outputsController.h"

using namespace sbc_pdu::restApi;

std::map<uint8_t, Output*> *OutputsController::outputs = nullptr;

OutputsController::OutputsController(std::map<uint8_t, Output*> *outputs) {
	OutputsController::outputs = outputs;
	this->getHandler = {
		.uri = "/api/v1/outputs",
		.method = HTTP_GET,
		.handler = &OutputsController::get,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	this->switchOutputHandler = {
		.uri = "/api/v1/outputs/switch",
		.method = HTTP_POST,
		.handler = &OutputsController::switchOutput,
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
}

void OutputsController::registerEndpoints(const httpd_handle_t &server) {
	httpd_register_uri_handler(server, &this->getHandler);
	httpd_register_uri_handler(server, &this->switchOutputHandler);
}

esp_err_t OutputsController::get(httpd_req_t *request) {
	sbc_pdu::restApi::Cors::addHeaders(request);
	restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
	if (!authenticator.authenticate(request)) {
		return ESP_OK;
	}
	httpd_resp_set_type(request, "application/json");
	cJSON *root = cJSON_CreateArray();
	for (const auto& outputPair : *OutputsController::outputs) {
		cJSON *outputObject = cJSON_CreateObject();
		Output *output = outputPair.second;
		if (output == nullptr) {
			ESP_LOGE("HTTP Outputs", "Output with ID %d is null.", outputPair.first);
			continue;
		}
		cJSON_AddNumberToObject(outputObject, "index", output->getIndex());
		cJSON_AddBoolToObject(outputObject, "alert", output->hasAlert());
		cJSON_AddBoolToObject(outputObject, "enabled", output->isEnabled());
		cJSON_AddNumberToObject(outputObject, "current", fabs(output->readCurrent()));
		cJSON_AddNumberToObject(outputObject, "voltage", output->readVoltage());
		cJSON_AddItemToArray(root, outputObject);
	}
	const char *response = cJSON_PrintUnformatted(root);
	httpd_resp_sendstr(request, response);
	delete response;
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t OutputsController::switchOutput(httpd_req_t *request) {
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
	cJSON *outputId = cJSON_GetObjectItem(root, "output");
	if (outputId == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"output\" property.");
	}
	if (!cJSON_IsNumber(outputId)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"output\" is not a number.");
	}
	auto output = OutputsController::outputs->find(outputId->valueint);
	if (output == OutputsController::outputs->end()) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Output with given ID does not exist.");
	}
	cJSON *state = cJSON_GetObjectItem(root, "state");
	if (state == nullptr) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Missing \"state\" property.");
	}
	if (!cJSON_IsBool(state)) {
		valid = false;
		RestApiUtils::createBadRequestResponse(request, "Property \"state\" is not a boolean.");
	}
	if (valid) {
		output->second->enable(state->valueint);
	}
	httpd_resp_sendstr(request, nullptr);
	cJSON_Delete(root);
	return ESP_OK;
}
