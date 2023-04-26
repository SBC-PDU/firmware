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
#include <esp_netif_net_stack.h>

#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/dhcp6.h>

#include "network/hostname.h"
#include "nvsManager.h"
#include "utils/interfaceUtils.h"

/**
 * WiFi Access Point information
 */
class WifiApInfo {
	public:
		/**
		 * Constructor
		 * @param info Info structure returned from scan
		 * @param inUse Is in use?
		 */
		explicit WifiApInfo(const wifi_ap_record_t &info, bool inUse = false);

		/**
		 * Is the AP in use?
		 * @return true Unit is connected to this AP.
		 * @return false Unit is not connected to this AP.
		 */
		bool isInUse();

		/**
		 * Returns auth mode
		 * @return Auth mode
		 */
		std::string getAuthMode();

		/**
		 * Returns BSSID (MAC address)
		 * @return BSSID
		 */
		uint8_t *getBssid();

		/**
		 * Returns SSID
		 * @return SSID
		 */
		const std::string &getSsid();

		/**
		 * Returns the RSSI
		 * @return RSSI
		 */
		int8_t getRssi();

	private:
		/// SSID
		std::string ssid;
		/// BSSID
		uint8_t bssid[6];
		/// Channel
		uint8_t channel;
		/// RSSI
		int8_t rssi;
		/// Auth mode
		wifi_auth_mode_t authmode;
		/// Is in use?
		bool inUse;
};

/**
 * WiFi configuration
 */
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

		/**
		 * Returns pre-shared key
		 * @return Pre-shared key
		 */
		const std::string &getPsk();

		/**
		 * Returns SSID
		 * @return SSID
		 */
		const std::string &getSsid();

	private:
		/// WiFi configuration
		wifi_config_t config;
		/// NVS manager
		NvsManager nvs = NvsManager("wifi");
		/// SSID
		std::string ssid;
		/// PSK
		std::string psk;
};

/**
 * WiFi station
 */
class Wifi {
	public:
		/**
		 * Constructor
		 * @param hostnameManager Hostname manager
		 */
		explicit Wifi(HostnameManager *hostnameManager);

		/**
		 * Desctructor
		 */
		~Wifi();

		/**
		 * Event handler
		 * @param arg Event handler argument, aside from event data
		 * @param base Event base ID
		 * @param id Event ID
		 * @param data Event data
		 */
		static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);

		/**
		 * Returns the primary MAC address as string
		 * @param separator Separator (e.g. `:`)
		 * @return Formatted MAC address
		 */
		static std::string getPrimaryMacAddress(const std::string &separator = "");

		/**
		 * Scans acccess points
		 * @return Access point list
		 */
		std::vector<WifiApInfo> scan();

		/// Auth modes
		static std::map<wifi_auth_mode_t, std::string> authModes;

	protected:
		/// FreeRTOS event group to signal when we are connected
		static EventGroupHandle_t eventGroup;
		/// NETIF
		static esp_netif_t *netifSta;
		/// Reconnection retries
		static uint8_t retries;
		/// Logger tag
		static constexpr const char *TAG = "WiFi STA";

	private:
		/// WiFi configuration manager
		WifiConfig configManager = WifiConfig();
		/// WiFi driver config
		wifi_config_t config;
		/// WiFi event handler
		esp_event_handler_instance_t eventHandlerWifiAny;
		/// IPv4 event handler
		esp_event_handler_instance_t eventHandlerIpGotIpv4;
		/// IPv6 event handler
		esp_event_handler_instance_t eventHandlerIpGotIpv6;
		/// Hostname manager
		HostnameManager *hostnameManager;
		/// WiFi init config
		wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
		/// Station WiFI mode
		wifi_mode_t mode = WIFI_MODE_STA;
		/// Maximal reconnection retries
		static const uint8_t maxRetries = 5;
};
