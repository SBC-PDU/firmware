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
#pragma once

#include <functional>
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include <cJSON.h>

#include "nvsManager.h"
#include "utils/base64.h"

namespace sbc_pdu {
	namespace restApi {
		/**
		 * HTTP Basic authenticator
		 */
		class BasicAuthenticator {
			public:
				/**
				 * Constructor
				 */
				explicit BasicAuthenticator();

				/**
				 * Returns Expected HTTP Authorization header
				 * @return std::string Expected HTTP Authorization header
				 */
				std::string getExpectedAuthorizationHeader();

				/**
				 * Authenticate the user
				 * @param request HTTP request
				 * @return bool Authentication status
				 */
				bool authenticate(httpd_req_t *request);

				/**
				 * Creates Unauthorized HTTP Response
				 * @param request HTTP request
				 */
				void createUnauthorizedResponse(httpd_req_t *request);

			private:
				/// Logger tag
				static constexpr const char *TAG = "HTTP auth";
				/// Expected Authorizaton HTTP header
				std::string expectedAuthorizationHeader;
				///  Username
				std::string username;
				/// Password
				std::string password;
				/// NVS manager
				NvsManager nvs = NvsManager("httpCredentials");
		};
	}
}
