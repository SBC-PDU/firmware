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

#include <string>

#include <esp_err.h>
#include <esp_http_server.h>
#include <cJSON.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/**
 * REST API utils
 */
class RestApiUtils {
	public:
		/**
		 * Creates bad request (error code 400) HTTP response
		 * @param request HTTP request
		 * @param reason Reason
		 */
		static void createBadRequestResponse(httpd_req_t *request, std::string reason) {
			httpd_resp_set_type(request, "text/plain");
			httpd_resp_set_hdr(request, "Connection", "keep-alive");
			httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, reason.c_str());
		}

		/**
		 * Parses JSON request
		 *
		 * @param request HTTP request
		 * @param response JSON root
		 * @return Execution status
		 */
		static esp_err_t parseJsonRequest(httpd_req_t *request, cJSON **response) {
			char content[1024];

			// Truncate if content length larger than the buffer
			size_t recvSize = MIN(request->content_len, sizeof(content));

			int ret = httpd_req_recv(request, content, recvSize);
			if (ret <= 0) {
				// Check if timeout occurred
				if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
					httpd_resp_send_408(request);
				}
				// In case of error, returning ESP_FAIL will ensure that the underlying socket is closed
				return ESP_FAIL;
			}

			*response = cJSON_Parse(content);
			return ESP_OK;
		}

};
