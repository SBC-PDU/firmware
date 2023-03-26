#include "network/http.h"

static const char *REST_TAG = "HTTP(S) server";

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath) {
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "image/svg+xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
esp_err_t HttpServer::getFrontendFiles(httpd_req_t *req) {
    rest_server_context_t *context = reinterpret_cast<rest_server_context_t *>(req->user_ctx);
    std::string path(context->basePath);
    std::string uri(req->uri);
    if (uri == "/index.html") {
        httpd_resp_set_status(req, "301 Moved Permanently");
        httpd_resp_set_hdr(req, "Location", "/");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    } else if (uri == "/") {
        path += "/index.html";
    } else {
        path += uri;
    }

    int fd = open(path.c_str(), O_RDONLY, 0);
    if (fd == -1) {
        if (uri.rfind("/api/", 0) == 0) {
            ESP_LOGE(REST_TAG, "File \"%s\" not found", path.c_str());
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
            return ESP_FAIL;
        }
        path = std::string(context->basePath) + "/index.html";
        fd = open(path.c_str(), O_RDONLY, 0);
        if (fd == -1) {
            ESP_LOGE(REST_TAG, "File \"%s\" not found", path.c_str());
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
            return ESP_FAIL;
        }
    }

    set_content_type_from_file(req, path.c_str());

    char *chunk = context->scratch;
    ssize_t readBytes;
    do {
        /* Read file in chunks into the scratch buffer */
        readBytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (readBytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file \"%s\"", path.c_str());
        } else if (readBytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, readBytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (readBytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File \"%s\" sending complete", path.c_str());
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

HttpServer::HttpServer(const std::string &basePath) {
    this->context->basePath = basePath;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    esp_err_t result = httpd_start(&this->handle, &config);
    if (result != ESP_OK) {
        ESP_LOGE(REST_TAG, "Server start failed. Reason: %d - %s", result, esp_err_to_name(result));
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
        .user_ctx = this->context
    };
    httpd_register_uri_handler(this->handle, &common_get_uri);
}
