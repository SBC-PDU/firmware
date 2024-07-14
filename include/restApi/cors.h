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

#include <esp_err.h>
#include <esp_http_server.h>


namespace sbc_pdu {
	namespace restApi {
		/**
		 * Cross-Origin Resource Sharing (CORS) helper
		 */
		class Cors {
			public:
				/**
				 * Adds CORS headers to HTTP response
				 * @param request HTTP request
				 */
				static void addHeaders(httpd_req_t *request);
		};
	}
}
