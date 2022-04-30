#include "network/multicast_dns.h"

const char TAG[] = "mDNS";

MulticastDns::MulticastDns(HostnameManager *hostnameManager): hostnameManager(hostnameManager) {
    esp_err_t result = mdns_init();
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Init failed: %d - %s", result, esp_err_to_name(result));
    } else {
        mdns_hostname_set(this->hostnameManager->get().c_str());
        mdns_instance_name_set("SBC PDU");
        mdns_service_add("HTTP server", "_http", "_tcp", 80, NULL, 0);
    }
}
