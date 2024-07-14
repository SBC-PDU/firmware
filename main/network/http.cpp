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
 *
 * This code is based on CC0 licensed example from https://github.com/espressif/esp-idf/blob/release/v5.1/examples/protocols/http_server/restful_server/main/rest_server.c
 */
#include "network/http.h"

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

esp_err_t HttpServer::setContentTypeFromFileExtension(httpd_req_t *request, const char *filePath) {
	const char *type = "text/plain";
	if (CHECK_FILE_EXTENSION(filePath, ".html")) {
		type = "text/html";
	} else if (CHECK_FILE_EXTENSION(filePath, ".js")) {
		type = "application/javascript";
	} else if (CHECK_FILE_EXTENSION(filePath, ".css")) {
		type = "text/css";
	} else if (CHECK_FILE_EXTENSION(filePath, ".png")) {
		type = "image/png";
	} else if (CHECK_FILE_EXTENSION(filePath, ".ico")) {
		type = "image/x-icon";
	} else if (CHECK_FILE_EXTENSION(filePath, ".svg")) {
		type = "image/svg+xml";
	}
	return httpd_resp_set_type(request, type);
}

esp_err_t HttpServer::getFrontendFiles(httpd_req_t *request) {
	rest_server_context_t *context = reinterpret_cast<rest_server_context_t *>(request->user_ctx);
	std::string path(context->basePath);
	std::string uri(request->uri);
	if (uri == "/index.html") {
		httpd_resp_set_status(request, "301 Moved Permanently");
		httpd_resp_set_hdr(request, "Location", "/");
		httpd_resp_send(request, NULL, 0);
		return ESP_OK;
	} else if (uri == "/") {
		path += "/index.html";
	} else {
		path += uri;
	}

	int fd = open(path.c_str(), O_RDONLY, 0);
	if (fd == -1) {
		if (uri.rfind("/api/", 0) == 0) {
			ESP_LOGE(TAG, "File \"%s\" not found", path.c_str());
			httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "File not found");
			return ESP_FAIL;
		}
		path = std::string(context->basePath) + "/index.html";
		fd = open(path.c_str(), O_RDONLY, 0);
		if (fd == -1) {
			ESP_LOGE(TAG, "File \"%s\" not found", path.c_str());
			httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "File not found");
			return ESP_FAIL;
		}
	}

	HttpServer::setContentTypeFromFileExtension(request, path.c_str());

	char *chunk = context->scratch;
	ssize_t readBytes;
	do {
		// Read file in chunks into the scratch buffer
		readBytes = read(fd, chunk, SCRATCH_BUFSIZE);
		if (readBytes == -1) {
			ESP_LOGE(TAG, "Failed to read file \"%s\"", path.c_str());
		} else if (readBytes > 0) {
			/* Send the buffer contents as HTTP response chunk */
			if (httpd_resp_send_chunk(request, chunk, readBytes) != ESP_OK) {
				close(fd);
				ESP_LOGE(TAG, "File sending failed!");
				// Abort sending file
				httpd_resp_sendstr_chunk(request, NULL);
				// Respond with 500 Internal Server Error
				httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
				return ESP_FAIL;
			}
		}
	} while (readBytes > 0);
	// Close file after sending complete
	close(fd);
	ESP_LOGI(TAG, "File \"%s\" sending complete", path.c_str());
	// Respond with an empty chunk to signal HTTP response completion
	httpd_resp_send_chunk(request, NULL, 0);
	return ESP_OK;
}

HttpServer::HttpServer(const std::string &basePath) {
	this->context->basePath = basePath;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 24;
	config.uri_match_fn = httpd_uri_match_wildcard;

	ESP_LOGI(TAG, "Starting HTTP Server");
	esp_err_t result = httpd_start(&this->handle, &config);
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "Server start failed. Reason: %d - %s", result, esp_err_to_name(result));
		delete context;
		throw std::exception();
	}
}

httpd_handle_t HttpServer::getHandle() {
	return this->handle;
}

void HttpServer::registerFrontendHandler() {
		httpd_uri_t common_get_uri = {
		.uri = "/*",
		.method = HTTP_GET,
		.handler = &HttpServer::getFrontendFiles,
		.user_ctx = this->context,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	httpd_register_uri_handler(this->handle, &common_get_uri);
}

void HttpServer::registerCorsHandler() {
	httpd_uri_t cors_get_uri = {
		.uri = "/api/*",
		.method = HTTP_OPTIONS,
		.handler = [](httpd_req_t *request) {
			sbc_pdu::restApi::Cors::addHeaders(request);
			httpd_resp_send(request, NULL, 0);
			return ESP_OK;
		},
		.user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = false,
		.handle_ws_control_frames = false,
		.supported_subprotocol = nullptr,
#endif
	};
	httpd_register_uri_handler(this->handle, &cors_get_uri);
}
