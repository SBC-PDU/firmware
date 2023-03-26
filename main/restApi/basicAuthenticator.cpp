#include "restApi/basicAuthenticator.h"

using namespace sbc_pdu::restApi;

static const char *TAG = "HTTP Auth";

BasicAuthenticator::BasicAuthenticator() {
    this->nvs.getString("username", this->username);
    this->nvs.getString("password", this->password);
    this->expectedAuthorizationHeader = this->getExpectedAuthorizationHeader();
}

bool BasicAuthenticator::authenticate(httpd_req_t *request) {
    size_t buf_len = httpd_req_get_hdr_value_len(request, "Authorization");
    if (buf_len == 0) {
        ESP_LOGE(TAG, "No auth header received");
        this->createUnauthorizedResponse(request);
        return false;
    }
    char *buf = new char[buf_len + 1];
    if (!buf) {
        ESP_LOGE(TAG, "No enough memory for basic authorization");
        httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Server Error");
        return false;
    }

    if (httpd_req_get_hdr_value_str(request, "Authorization", buf, buf_len + 1) == ESP_OK) {
        ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
    } else {
        ESP_LOGE(TAG, "No auth value received");
        this->createUnauthorizedResponse(request);
        delete[] buf;
        return false;
    }
    std::string actualHeader = std::string(buf);
    int compare = this->expectedAuthorizationHeader.compare(actualHeader);
    if (compare != 0) {
        this->createUnauthorizedResponse(request);
        delete[] buf;
        return false;
    }
    ESP_LOGI(TAG, "Authenticated!");
    delete[] buf;
    return true;
}

void BasicAuthenticator::createUnauthorizedResponse(httpd_req_t *request) {
    httpd_resp_set_type(request, "application/json");
    httpd_resp_set_hdr(request, "Connection", "keep-alive");
    httpd_resp_set_hdr(request, "WWW-Authenticate", "Basic realm=\"SBC-PDU\"");
    httpd_resp_send_err(request, HTTPD_401_UNAUTHORIZED, "Unauthorized");
}

std::string BasicAuthenticator::getExpectedAuthorizationHeader() {
    size_t out;
    size_t n = 0;
    std::string credentials = this->username + ":" + this->password;
    const unsigned char* bufferToEncode = reinterpret_cast<const unsigned char*>(credentials.c_str());
    // Calculate buffer size
    esp_crypto_base64_encode(nullptr, 0, &n, bufferToEncode, credentials.length());
    char *digest = new char[n + 1];
    if (digest) {
        // Encode base64 from string username:password
        esp_crypto_base64_encode(reinterpret_cast<unsigned char*>(digest), n, &out, bufferToEncode, credentials.length());
    } else {
        return "";
    }
    std::string header = std::string("Basic ") + digest;
    delete[] digest;
    return header;
}
