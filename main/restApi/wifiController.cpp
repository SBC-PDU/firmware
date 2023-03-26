#include "restApi/wifiController.h"

using namespace sbc_pdu::restApi;

WifiController::WifiController(Wifi *manager) {
    this->scanHandler = {
        .uri = "/api/v1/wifi/scan",
        .method = HTTP_GET,
        .handler = &WifiController::scan,
        .user_ctx = manager,
    };
}

void WifiController::registerEndpoints(const httpd_handle_t &server) {
    httpd_register_uri_handler(server, &this->scanHandler);
}

esp_err_t WifiController::scan(httpd_req_t *request) {
    sbc_pdu::restApi::BasicAuthenticator authenticator = sbc_pdu::restApi::BasicAuthenticator();
    if (!authenticator.authenticate(request)) {
        return ESP_OK;
    }
    Wifi *manager = reinterpret_cast<Wifi*>(request->user_ctx);
    httpd_resp_set_type(request, "application/json");
    cJSON *root = cJSON_CreateArray();
    std::vector<WifiApInfo> list = manager->scan();
    for (WifiApInfo &apInfo : list) {
        cJSON *ap = cJSON_CreateObject();
        cJSON_AddStringToObject(ap, "authMode", apInfo.getAuthMode().c_str());
        cJSON_AddStringToObject(ap, "bssid", InterfaceUtils::macToString(apInfo.getBssid(), ":").c_str());
        cJSON_AddBoolToObject(ap, "inUse", apInfo.isInUse());
        cJSON_AddNumberToObject(ap, "rssi", apInfo.getRssi());
        cJSON_AddStringToObject(ap, "ssid", apInfo.getSsid().c_str());
        cJSON_AddItemToArray(root, ap);
    }
    const char *response = cJSON_PrintUnformatted(root);
    httpd_resp_sendstr(request, response);
    delete response;
    cJSON_Delete(root);
    return ESP_OK;
}

