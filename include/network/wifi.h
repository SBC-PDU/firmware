#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <esp_log.h>
#include <esp_smartconfig.h>

#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/dhcp6.h>

#include "network/hostname.h"
#include "nvs_manager.h"
#include "interfaceUtils.h"

class WifiApInfo {
    public:
        explicit WifiApInfo(const wifi_ap_record_t &info, bool inUse = false);
        bool isInUse();
        std::string getAuthMode();
        uint8_t *getBssid();
        const std::string &getSsid();
        int8_t getRssi();
    private:
        std::string ssid;
        uint8_t bssid[6];
        uint8_t channel;
        int8_t rssi;
        wifi_auth_mode_t authmode;
        bool inUse;
};

class WifiConfig {
    public:
        /**
         * Constructor
         */
        WifiConfig();

        /**
         * Returns WiFi configuration
         * @return WiFi configuration
         */
        const wifi_config_t &get();

        const std::string &getPsk();

        const std::string &getSsid();
    private:
        /**
         * WiFi configuration
         */
        wifi_config_t config;

        /**
         * NVS manager
         */
        NvsManager nvs = NvsManager("wifi");

        /**
         * SSID
         */
        std::string ssid;

        /**
         * PSK
         */
        std::string psk;
};

class Wifi {
    public:
        /**
         * Constructor
         */
        explicit Wifi(HostnameManager *hostnameManager);
        ~Wifi();
        static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);
        static std::string getPrimaryMacAddress(const std::string &separator = "");
        std::vector<WifiApInfo> scan();
        static std::map<wifi_auth_mode_t, std::string> authModes;

    protected:
        /**
         * FreeRTOS event group to signal when we are connected
         */
        static EventGroupHandle_t eventGroup;
        static esp_netif_t *netifSta;
        static uint8_t retries;
        /// Logger tag
        static constexpr const char *TAG = "WiFi STA";
    
    private:
        WifiConfig configManager = WifiConfig();
        wifi_config_t config;
        esp_event_handler_instance_t eventHandlerWifiAny;
        esp_event_handler_instance_t eventHandlerIpGotIpv4;
        esp_event_handler_instance_t eventHandlerIpGotIpv6;
        HostnameManager *hostnameManager;
        wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
        wifi_mode_t mode = WIFI_MODE_STA;
        static const uint8_t maxRetries = 5;
};
