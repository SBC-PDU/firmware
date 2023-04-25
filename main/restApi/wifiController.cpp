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
#include "restApi/wifiController.h"

using namespace sbc_pdu::restApi;

WifiController::WifiController(Wifi *manager) {
    this->getConfigHandler = {
        .uri = "/api/v1/wifi",
        .method = HTTP_GET,
        .handler = &WifiController::getConfig,
        .user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
#endif
    };
    this->putConfigHandler = {
        .uri = "/api/v1/wifi",
        .method = HTTP_PUT,
        .handler = &WifiController::putConfig,
        .user_ctx = nullptr,
#ifdef CONFIG_HTTPD_WS_SUPPORT
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
#endif
    };
    this->scanHandler = {
        .uri = "/api/v1/wifi/scan",
        .method = HTTP_GET,
        .handler = &WifiController::scan,
        .user_ctx = manager,
#ifdef CONFIG_HTTPD_WS_SUPPORT
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
#endif
    };
}

void WifiController::registerEndpoints(const httpd_handle_t &server) {
    httpd_register_uri_handler(server, &this->getConfigHandler);
    httpd_register_uri_handler(server, &this->putConfigHandler);
    httpd_register_uri_handler(server, &this->scanHandler);
}

esp_err_t WifiController::getConfig(httpd_req_t *request) {
    sbc_pdu::restApi::BasicAuthenticator authenticator = sbc_pdu::restApi::BasicAuthenticator();
    if (!authenticator.authenticate(request)) {
        return ESP_OK;
    }
    NvsManager nvs = NvsManager("wifi");
    httpd_resp_set_type(request, "application/json");
    cJSON *root = cJSON_CreateObject();
    uint8_t authModeValue;
    if (nvs.get("authMode", authModeValue) != ESP_OK) {
        authModeValue = static_cast<uint8_t>(WIFI_AUTH_OPEN);
    }
    auto authMode = Wifi::authModes.find(
        static_cast<wifi_auth_mode_t>(authModeValue)
    );
    std::string authModeString = authMode != Wifi::authModes.end() ? authMode->second : "unknown";
    cJSON_AddStringToObject(root, "authMode", authModeString.c_str());
    std::string ssid;
    if (nvs.getString("ssid", ssid) != ESP_OK) {
        ssid = "";
    }
    cJSON_AddStringToObject(root, "ssid", ssid.c_str());
    std::string psk;
    if (nvs.getString("psk", psk) != ESP_OK) {
        psk = "";
    }
    cJSON_AddStringToObject(root, "psk", psk.c_str());
    const char *response = cJSON_PrintUnformatted(root);
    httpd_resp_sendstr(request, response);
    delete response;
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WifiController::putConfig(httpd_req_t *request) {
    restApi::BasicAuthenticator authenticator = restApi::BasicAuthenticator();
    if (!authenticator.authenticate(request)) {
        return ESP_OK;
    }
    cJSON *root = nullptr;
    esp_err_t result = RestApiUtils::parseJsonRequest(request, &root);
    if (result != ESP_OK) {
        return result;
    }
    NvsManager nvs = NvsManager("wifi");
    cJSON *authMode = cJSON_GetObjectItem(root, "authMode");
    if (authMode == nullptr) {
        RestApiUtils::createBadRequestResponse(request, "Missing \"authMode\" property.");

    }
    if (!cJSON_IsString(authMode)) {
        RestApiUtils::createBadRequestResponse(request, "Property \"authMode\" is not a string.");
    }
    auto authModeValue = std::find_if(
        Wifi::authModes.begin(),
        Wifi::authModes.end(),
        [authMode](const std::pair<wifi_auth_mode_t, std::string> &pair) {
            return pair.second == authMode->valuestring;
        }
    );
    if (authModeValue == Wifi::authModes.end()) {
        RestApiUtils::createBadRequestResponse(request, "Unknown \"authMode\" value.");
    }
    nvs.set("authmode", static_cast<uint8_t>(authModeValue->first));
    cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
    if (ssid == nullptr) {
        RestApiUtils::createBadRequestResponse(request, "Missing \"ssid\" property.");
    }
    if (!cJSON_IsString(ssid)) {
        RestApiUtils::createBadRequestResponse(request, "Property \"ssid\" is not a string.");
    }
    nvs.setString("ssid", ssid->valuestring);
    cJSON *psk = cJSON_GetObjectItem(root, "psk");
    if (psk == nullptr) {
        RestApiUtils::createBadRequestResponse(request, "Missing \"psk\" property.");
    }
    if (!cJSON_IsString(psk)) {
        RestApiUtils::createBadRequestResponse(request, "Property \"psk\" is not a string.");
    }
    nvs.setString("psk", psk->valuestring);
    nvs.commit();
    httpd_resp_sendstr(request, nullptr);
    cJSON_Delete(root);
    return ESP_OK;
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

