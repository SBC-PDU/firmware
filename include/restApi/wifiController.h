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
#pragma once

#include <functional>
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>

#include <cJSON.h>

#include "network/wifi.h"
#include "restApi/basicAuthenticator.h"
#include "restApi/cors.h"
#include "utils/interfaceUtils.h"
#include "utils/restApiUtils.h"

namespace sbc_pdu {
	namespace restApi {
		/**
		 * WiFi manager REST API endpoints
		 */
		class WifiController {
			public:
				/**
				 * Constructor
				 */
				explicit WifiController(Wifi *manager);

				/**
				 * Registers the endpoints
				 * @param server HTTP server handle
				 */
				void registerEndpoints(const httpd_handle_t &server);

				/**
				 * Scans the available WiFi APs
				 * @param request HTTP request
				 */
				static esp_err_t scan(httpd_req_t *request);

				/**
				 * Returns the WiFi config
				 * @param request HTTP request
				 */
				static esp_err_t getConfig(httpd_req_t *request);

				/**
				 * Updates the WiFi config
				 * @param request HTTP request
				 */
				static esp_err_t putConfig(httpd_req_t *request);

			private:
				/// Retrieve WiFi config endpoint handler
				httpd_uri_t getConfigHandler;
				/// Update WiFi config endpoint handler
				httpd_uri_t putConfigHandler;
				/// Scan endpoint handler
				httpd_uri_t scanHandler;
		};
	}
}
