#include "network/hostname.h"
#include "network/wifi.h"

std::string HostnameManager::get() {
    std::string hostname;
    esp_err_t result = this->nvs.getString("hostname", hostname);
    if (result != ESP_OK) {
        hostname = "sbc-pdu_" + Wifi::getPrimaryMacAddress();
        this->set(hostname);
    }
    return hostname;
}

void HostnameManager::set(const std::string &hostname) {
    ESP_ERROR_CHECK(this->nvs.setString("hostname", hostname));
}