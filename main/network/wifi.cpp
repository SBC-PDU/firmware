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
#include "network/wifi.h"

std::map<wifi_auth_mode_t, std::string> Wifi::authModes = {
    {WIFI_AUTH_OPEN, "open"},
    {WIFI_AUTH_WEP, "WEP"},
    {WIFI_AUTH_WPA_PSK, "WPA-Personal"},
    {WIFI_AUTH_WPA2_PSK, "WPA2-Personal"},
    {WIFI_AUTH_WPA_WPA2_PSK, "WPA/WPA2-Personal"},
    {WIFI_AUTH_WPA2_ENTERPRISE, "WPA2-Enterprise"},
    {WIFI_AUTH_WPA3_PSK, "WPA3-Personal"},
    {WIFI_AUTH_WPA2_WPA3_PSK, "WPA2/WPA3-Personal"},
    {WIFI_AUTH_WAPI_PSK, "WAPI-PSK"},
    {WIFI_AUTH_OWE, "OWE"},
};

WifiApInfo::WifiApInfo(const wifi_ap_record_t &info, bool inUse) : inUse(inUse) {
    this->authmode = info.authmode;
    memcpy(this->bssid, info.bssid, sizeof(this->bssid));
    this->channel = info.primary;
    this->ssid = std::string(reinterpret_cast<const char*>(info.ssid));
    this->rssi = info.rssi;
}

bool WifiApInfo::isInUse() {
    return this->inUse;
}

std::string WifiApInfo::getAuthMode() {
    auto mode = Wifi::authModes.find(this->authmode);
    if (mode == Wifi::authModes.end()) {
        ESP_LOGE("WiFi", "Unknown auth mode: %d", static_cast<int>(this->authmode));
        return "unknown";
    }
    return mode->second;
}

uint8_t *WifiApInfo::getBssid() {
    return this->bssid;
}

int8_t WifiApInfo::getRssi() {
    return this->rssi;
}

const std::string &WifiApInfo::getSsid() {
    return this->ssid;
}

WifiConfig::WifiConfig() {
    memset(&this->config, 0, sizeof(wifi_config_t));
    nvs.getString("ssid", this->ssid);
    memcpy(this->config.sta.ssid, this->ssid.c_str(), this->ssid.length());
    nvs.getString("psk", this->psk);
    memcpy(this->config.sta.password, this->psk.c_str(), this->psk.length());
    uint8_t authmode;
    nvs.get("authmode", authmode);
    this->config.sta.threshold.authmode = static_cast<wifi_auth_mode_t>(authmode);
    this->config.sta.pmf_cfg.capable = true;
    this->config.sta.pmf_cfg.required = false;
}

const wifi_config_t &WifiConfig::get() {
    return this->config;
}

const std::string &WifiConfig::getPsk() {
    return this->psk;
}

const std::string &WifiConfig::getSsid() {
    return this->ssid;
}

EventGroupHandle_t Wifi::eventGroup;
esp_netif_t *Wifi::netifSta = nullptr;
uint8_t Wifi::retries = 0;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

void Wifi::eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Connected to the AP.");
#if CONFIG_LWIP_IPV6
        struct netif *netif = reinterpret_cast<struct netif*>(esp_netif_get_netif_impl(Wifi::netifSta));
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(Wifi::netifSta));
        dhcp6_enable_stateful(netif);
        dhcp6_enable_stateless(netif);
#endif
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = reinterpret_cast<wifi_event_sta_disconnected_t*>(data);
        ESP_LOGE(TAG, "Disconnected from the AP (SSID: %s), reason: %d (%s).", event->ssid, event->reason, esp_err_to_name(event->reason));
        if (Wifi::retries < Wifi::maxRetries) {
            esp_wifi_connect();
            Wifi::retries++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        } else {
            xEventGroupSetBits(Wifi::eventGroup, WIFI_FAIL_BIT);
        }
        ESP_LOGE(TAG, "Unable to connect to the AP.");
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = reinterpret_cast<ip_event_got_ip_t*>(data);
        ESP_LOGI(TAG, "Got IPv4 address on interface \"%s\"", esp_netif_get_desc(event->esp_netif));
        ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "IPv4 netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "IPv4 gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        Wifi::retries = 0;
        xEventGroupSetBits(Wifi::eventGroup, WIFI_CONNECTED_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_GOT_IP6) {
#if CONFIG_LWIP_IPV6
        ip_event_got_ip6_t* event = reinterpret_cast<ip_event_got_ip6_t*>(data);
        if (event->esp_netif != Wifi::netifSta) {
            return;
        }
        esp_ip6_addr_type_t addressType = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
        ESP_LOGI(TAG, "Got IPv6 address on interface \"%s\": " IPV6STR, esp_netif_get_desc(event->esp_netif), IPV62STR(event->ip6_info.ip));
        if (addressType != ESP_IP6_ADDR_IS_GLOBAL) {
            return;
        }
        Wifi::retries = 0;
        xEventGroupSetBits(Wifi::eventGroup, WIFI_CONNECTED_BIT);
#endif
    }
}

Wifi::Wifi(HostnameManager *hostnameManager): hostnameManager(hostnameManager) {
    Wifi::eventGroup = xEventGroupCreate();
    Wifi::netifSta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_set_hostname(Wifi::netifSta, this->hostnameManager->get().c_str()));

    ESP_ERROR_CHECK(esp_wifi_init(&this->initConfig));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::eventHandler, NULL, &this->eventHandlerWifiAny));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::eventHandler, NULL, &this->eventHandlerIpGotIpv4));
#if CONFIG_LWIP_IPV6
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_GOT_IP6, &Wifi::eventHandler, NULL, &this->eventHandlerIpGotIpv6));
#endif

    this->config = this->configManager.get();
    wifi_country_t country = {
        .cc = "CZ",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&country));
    ESP_ERROR_CHECK(esp_wifi_set_mode(this->mode));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &this->config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi station inicialization finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(Wifi::eventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to the AP with SSID: %s", this->configManager.getSsid().c_str());
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to the AP SSID: %s, PSK: %s", this->configManager.getSsid().c_str(), this->configManager.getPsk().c_str());
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

Wifi::~Wifi() {
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, this->eventHandlerIpGotIpv4));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_GOT_IP6, this->eventHandlerIpGotIpv6));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, this->eventHandlerWifiAny));
    vEventGroupDelete(Wifi::eventGroup);
}

std::string Wifi::getPrimaryMacAddress(const std::string &separator) {
    uint8_t buffer[6] = {0, 0, 0, 0, 0, 0};
    ESP_ERROR_CHECK(esp_read_mac(buffer, ESP_MAC_WIFI_STA));
    return InterfaceUtils::macToString(buffer, separator);
}

std::vector<WifiApInfo> Wifi::scan() {
    uint16_t count = 0;
    uint16_t number = 16;
    wifi_ap_record_t *info = new wifi_ap_record_t[number];
    esp_wifi_scan_start(nullptr, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&count));
    std::vector<WifiApInfo> list;
    wifi_ap_record_t currentInfo;
    bool isConnected = esp_wifi_sta_get_ap_info(&currentInfo) == ESP_OK;
    for (int i = 0; (i < number) && (i < count); i++) {
        bool inUse = false;
        if (isConnected) {
            inUse = memcmp(currentInfo.bssid, info[i].bssid, 6) == 0;
        }
        WifiApInfo ap = WifiApInfo(info[i], inUse);
        list.push_back(ap);
    }
    delete[] info;
    return list;
}
