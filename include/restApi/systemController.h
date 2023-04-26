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

#include <esp_chip_info.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_timer.h>
#include <esp_netif.h>

#include <cJSON.h>

#include "network/wifi.h"
#include "restApi/basicAuthenticator.h"
#include "utils/interfaceUtils.h"

namespace sbc_pdu {
	namespace restApi {
		/**
		 * System information REST API endpoints
		 */
		class SystemController {
			public:
				/**
				 * Constructor
				 */
				explicit SystemController();

				/**
				 * Registers the endpoints
				 * @param server HTTP server handle
				 */
				void registerEndpoints(const httpd_handle_t &server);

				/**
				 * Returns the system information
				 * @param request HTTP request
				 * @return Execution status
				 */
				static esp_err_t getInfo(httpd_req_t *request);

				/**
				 * Restarts the unitinsert_final_newline
				 * @param request HTTP request
				 * @return Execution status
				 */
				static esp_err_t restart(httpd_req_t *request);

				/**
				 * Sets the Chip Info object to JSON response
				 * @param root JSON root object
				 */
				static void setChipInfo(cJSON *root);

				/**
				 * Sets the Network Info object to JSON response
				 * @param root JSON root object
				 */
				static void setNetworkInfo(cJSON *root);

			private:
				/// Get system info endpoint handler
				httpd_uri_t getInfoHandler;
				/// Restart endpoint handler
				httpd_uri_t restartHandler;
		};
	}
}
