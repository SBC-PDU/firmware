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

#include <cJSON.h>

#include "network/hostname.h"
#include "restApi/basicAuthenticator.h"
#include "utils/restApiUtils.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

namespace sbc_pdu {
	namespace restApi {
		/**
		 * Hostname manager REST API endpoints
		 */
		class HostnameController {
			public:
				/**
				 * Constructor
				 */
				explicit HostnameController(HostnameManager *hostnameManager);

				/**
				 * Registers the endpoints
				 * @param server HTTP server handle
				 */
				void registerEndpoints(const httpd_handle_t &server);

				/**
				 * Returns the hostname
				 * @param request HTTP request
				 */
				static esp_err_t get(httpd_req_t *request);

				/**
				 * Updates the hostname
				 * @param request HTTP request
				 */
				static esp_err_t put(httpd_req_t *request);

			private:
				/// Retrieve hostname endpoint handler
				httpd_uri_t getHandler;
				/// Update hostname endpoint handler
				httpd_uri_t putHandler;
		};
	}
}
