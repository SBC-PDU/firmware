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
#include "restApi/basicAuthenticator.h"

using namespace sbc_pdu::restApi;

static const char *TAG = "HTTP Auth";

BasicAuthenticator::BasicAuthenticator() {
	this->nvs.getString("username", this->username);
	this->nvs.getString("password", this->password);
	this->expectedAuthorizationHeader = this->getExpectedAuthorizationHeader();
}

bool BasicAuthenticator::authenticate(httpd_req_t *request) {
	const char headerName[] = "Authorization";
	size_t bufferLength = httpd_req_get_hdr_value_len(request, headerName);
	if (bufferLength == 0) {
		ESP_LOGE(TAG, "No Authorization header received");
		this->createUnauthorizedResponse(request);
		return false;
	}
	char *buffer = new char[bufferLength + 1];
	if (!buffer) {
		ESP_LOGE(TAG, "No enough memory for basic authorization");
		httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Server Error");
		return false;
	}

	if (httpd_req_get_hdr_value_str(request, headerName, buffer, bufferLength + 1) == ESP_OK) {
		ESP_LOGI(TAG, "Found header => Authorization: %s", buffer);
	} else {
		ESP_LOGE(TAG, "No Authorization header received");
		this->createUnauthorizedResponse(request);
		delete[] buffer;
		return false;
	}
	if (std::string(buffer) != this->expectedAuthorizationHeader) {
		this->createUnauthorizedResponse(request);
		delete[] buffer;
		return false;
	}
	ESP_LOGI(TAG, "Authenticated!");
	delete[] buffer;
	return true;
}

void BasicAuthenticator::createUnauthorizedResponse(httpd_req_t *request) {
	httpd_resp_set_type(request, "text/plain");
	httpd_resp_set_hdr(request, "Connection", "keep-alive");
	httpd_resp_set_hdr(request, "WWW-Authenticate", "Basic realm=\"SBC-PDU\"");
	httpd_resp_send_err(request, HTTPD_401_UNAUTHORIZED, "Unauthorized");
}

std::string BasicAuthenticator::getExpectedAuthorizationHeader() {
	std::string credentials = this->username + ":" + this->password;
	return std::string("Basic ") + Base64::encode(credentials);
}
