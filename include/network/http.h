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

#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    std::string basePath;
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


        static esp_err_t getFrontendFiles(httpd_req_t *request);

        void registerFrontendHandler();

    private:
        /// HTTP server handle
        httpd_handle_t handle = nullptr;
        /// HTTP server context
        rest_server_context_t *context = new rest_server_context_t;
};
