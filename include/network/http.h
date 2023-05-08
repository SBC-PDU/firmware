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

#include <string>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_tls_crypto.h>
#include <esp_http_server.h>

#include <fcntl.h>
#include <esp_err.h>
#include <esp_spiffs.h>
#include <esp_vfs.h>
#include <cJSON.h>

#include "restApi/cors.h"

#define SCRATCH_BUFSIZE (10240)

/**
 * REST API server context
 */
typedef struct rest_server_context {
	/// Base path
	std::string basePath;
	/// Scratch buffer
	char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

/**
 * HTTP server
 */
class HttpServer {
	public:
		/**
		 * Constructor
		 * @param basePath SPIFFS base path
		 */
		explicit HttpServer(const std::string &basePath);

		/**
		 * Returns the HTTP server handle
		 * @return HTTP server handle
		 */
		httpd_handle_t getHandle();

		/**
		 * Frontend handler
		 * @param request HTTP request
		 * @return Execution status
		 */
		static esp_err_t getFrontendFiles(httpd_req_t *request);

		/**
		 * Registers the frontend handler
		 */
		void registerFrontendHandler();

		/**
		 * Registers the CORS handler
		 */
		void registerCorsHandler();

		/**
		 * Set HTTP response content type according to file extension
		 * @param request HTTP request
		 * @param filePath File path
		 * @return Execution status
		 */
		static esp_err_t setContentTypeFromFileExtension(httpd_req_t *request, const char *filePath);

	protected:
		/// Logger tag
		static constexpr const char *TAG = "HTTP server";

	private:
		/// HTTP server handle
		httpd_handle_t handle = nullptr;
		/// HTTP server context
		rest_server_context_t *context = new rest_server_context_t;
};
