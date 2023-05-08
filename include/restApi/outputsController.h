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
#include <map>
#include <string>
#include <vector>

#include <esp_err.h>
#include <esp_http_server.h>

#include <cJSON.h>

#include "output.h"
#include "restApi/basicAuthenticator.h"
#include "restApi/cors.h"
#include "utils/restApiUtils.h"

namespace sbc_pdu {
	namespace restApi {
		/**
		 * Power outputs manager REST API endpoints
		 */
		class OutputsController {
			public:
				/**
				 * Constructor
				 */
				OutputsController(std::map<uint8_t, Output*> *outputs);

				/**
				 * Registers the endpoints
				 * @param server HTTP server handle
				 */
				void registerEndpoints(const httpd_handle_t &server);

				/**
				 * Returns information about zhe outputs
				 * @param request HTTP request
				 */
				static esp_err_t get(httpd_req_t *request);

				/**
				 * Switches on/off the output
				 * @param request HTTP request
				 */
				static esp_err_t switchOutput(httpd_req_t *request);

			private:
				/// Outputs
				static std::map<uint8_t, Output*> *outputs;
				/// Retrieve information about outputs endpoint handler
				httpd_uri_t getHandler;
				/// Switch output endpoint handler
				httpd_uri_t switchOutputHandler;
		};
	}
}
