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

#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>

#include <driver/gpio.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>

#include "network/hostname.h"

/**
 * Ethernet
 */
class Ethernet {
    public:
        /**
         * Constructor
         * @param hostnameManager Hostname manager
         */
        explicit Ethernet(HostnameManager *hostnameManager);

        /**
         * Event handler
         * @param arg Event handler argument, aside from event data
         * @param base Event base ID
         * @param id Event ID
         * @param data Event data
         */
        static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);
    protected:
        /// NETIF
        static esp_netif_t *netif;
        /// Logger tag
        static constexpr const char *TAG = "Ethernet";
    private:
        /// Ethernet driver handle
        esp_eth_handle_t handle = nullptr;
        /// NETIF config
        esp_netif_config_t netifConfig = ESP_NETIF_DEFAULT_ETH();
        /// Hostname manager
        HostnameManager *hostnameManager;
        /// MAC configuration
        eth_mac_config_t macConfig = ETH_MAC_DEFAULT_CONFIG();
        /// ESP32 Ethernet MAC configuration
        eth_esp32_emac_config_t emacConfig = ETH_ESP32_EMAC_DEFAULT_CONFIG();
        /// MAC
        esp_eth_mac_t *mac = nullptr;
        /// PHY config
        eth_phy_config_t phyConfig = ETH_PHY_DEFAULT_CONFIG();
        /// PHY
        esp_eth_phy_t *phy = nullptr;
};
