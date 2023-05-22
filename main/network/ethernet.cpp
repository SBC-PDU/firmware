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
#ifdef CONFIG_ETH_USE_ESP32_EMAC
#include "network/ethernet.h"


esp_netif_t *Ethernet::netif = nullptr;

Ethernet::Ethernet(HostnameManager *hostnameManager): hostnameManager(hostnameManager) {
	Ethernet::netif = esp_netif_new(&this->netifConfig);
	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_set_hostname(Ethernet::netif, this->hostnameManager->get().c_str()));

	this->emacConfig.smi_mdc_gpio_num = GPIO_NUM_23;
	this->emacConfig.smi_mdio_gpio_num = GPIO_NUM_18;
	this->emacConfig.clock_config.rmii.clock_mode = EMAC_CLK_OUT;
	this->emacConfig.clock_config.rmii.clock_gpio = EMAC_CLK_OUT_GPIO;
	this->mac = esp_eth_mac_new_esp32(&this->emacConfig, &this->macConfig);
	this->phyConfig.phy_addr = 0;
	this->phyConfig.reset_gpio_num = GPIO_NUM_17;

	this->phy = esp_eth_phy_new_lan87xx(&this->phyConfig);

	esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(this->mac, this->phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &this->handle));

	// Attach Ethernet driver to TCP/IP stack
	ESP_ERROR_CHECK(esp_netif_attach(Ethernet::netif, esp_eth_new_netif_glue(this->handle)));
	// Register user defined event handers
	ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &this->eventHandler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &this->eventHandler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &this->eventHandler, NULL));
	ESP_ERROR_CHECK(esp_netif_dhcpc_start(Ethernet::netif));
	// Start Ethernet driver
	ESP_ERROR_CHECK(esp_eth_start(this->handle));
}

void Ethernet::eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) {
	if (base == ETH_EVENT) {
		switch (id) {
			case ETHERNET_EVENT_CONNECTED: {
				ESP_LOGI(TAG, "Link up");
				ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(Ethernet::netif));
				break;
			}
			case ETHERNET_EVENT_DISCONNECTED:
				ESP_LOGI(TAG, "Link down");
				break;
		}
	} else if (base == IP_EVENT) {
		if (id == IP_EVENT_ETH_GOT_IP) {
			ip_event_got_ip_t* event = reinterpret_cast<ip_event_got_ip_t*>(data);
			ESP_LOGI(TAG, "Got IPv4 address on interface \"%s\"", esp_netif_get_desc(event->esp_netif));
			ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
			ESP_LOGI(TAG, "IPv4 netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
			ESP_LOGI(TAG, "IPv4 gateway: " IPSTR, IP2STR(&event->ip_info.gw));
		} else if (id == IP_EVENT_GOT_IP6) {
			ip_event_got_ip6_t* event = reinterpret_cast<ip_event_got_ip6_t*>(data);
			if (event->esp_netif != Ethernet::netif) {
				return;
			}
			ESP_LOGI(TAG, "Got IPv6 address: " IPV6STR, IPV62STR(event->ip6_info.ip));
		}
	}

}
#endif
