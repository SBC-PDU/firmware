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
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <map>
#include <vector>
#include <cJSON.h>

#include "homeAssistant.h"
#include "i2c_master.h"
#include "ina3221.h"
#if REVISION == 2
#include "network/ethernet.h"
#endif
#include "network/hostname.h"
#include "network/http.h"
#include "network/mdns.h"
#include "network/mqtt.h"
#include "network/sntp.h"
#include "network/wifi.h"
#include "nvsManager.h"
#include "mcp7940n.h"
#include "restApi/authController.h"
#include "restApi/basicAuthenticator.h"
#include "restApi/hostnameController.h"
#include "restApi/mqttController.h"
#include "restApi/ntpController.h"
#include "restApi/outputsController.h"
#include "restApi/systemController.h"
#include "restApi/wifiController.h"
#include "sbcPduManagement.h"
#include "output.h"
#include "spiffs.h"

extern "C" void app_main();

/// @brief Alert queue
static QueueHandle_t alertQueue = nullptr;
/// Home Assistant MQTT client
HomeAssistant *homeAssistant = nullptr;
/// @brief Pointer to SBC PDU management client instance
SbcPduManagement *pduManagement = nullptr;
/// @brief Output map <index, pointer to output>
std::map<uint8_t, Output*> outputs = {};

/**
 * MQTT connect callback
 * @param client MQTT client
 * @param base MQTT event base
 * @param event MQTT event
 */
void mqttConnectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event) {
	if (homeAssistant != nullptr) {
		homeAssistant->connectCallback(client, base, event);
	}
	if (pduManagement != nullptr) {
		pduManagement->connectCallback(client, base, event);
	}
}

/**
 * Alert GPIO interrupt handler
 * @param arg Interrupt argument - PDU output ID
 */
static void IRAM_ATTR gpioAlertHandler(void* arg) {
	uint32_t outputId = (uint32_t) arg;
	xQueueSendFromISR(alertQueue, &outputId, nullptr);
}


/**
 * Alert task - sends MQTT message when alert occurres
 * @param arg Task argument
 */
static void alertTask(void* arg) {
	uint32_t outputId;
	for (;;) {
		if (xQueueReceive(alertQueue, &outputId, portMAX_DELAY)) {
			auto output = outputs.find(static_cast<uint8_t>(outputId));
			if (output != outputs.end()) {
				if (pduManagement != nullptr) {
					pduManagement->publishOutputAlert(output->second);
				}
			}
		}
	}
}


/**
 * Initializes HTTP server
 */
void initHttp(Wifi *wifi, HostnameManager *hostnameManager) {
	std::string basePath = "/spiffs";
	SPIFFS mainSpiffs(basePath);
	HttpServer httpServer(basePath);
	httpd_handle_t httpdHandle = httpServer.getHandle();
	namespace restApi = sbc_pdu::restApi;
	restApi::AuthController auth = restApi::AuthController();
	auth.registerEndpoints(httpdHandle);
	restApi::WifiController wifiInfo = restApi::WifiController(wifi);
	wifiInfo.registerEndpoints(httpdHandle);
	restApi::SystemController systemInfo = restApi::SystemController();
	systemInfo.registerEndpoints(httpdHandle);
	restApi::HostnameController hostname = restApi::HostnameController(hostnameManager);
	hostname.registerEndpoints(httpdHandle);
	restApi::NtpController ntp = restApi::NtpController();
	ntp.registerEndpoints(httpdHandle);
	restApi::MqttController mqtt = restApi::MqttController();
	mqtt.registerEndpoints(httpdHandle);
	restApi::OutputsController outputsController = restApi::OutputsController(&outputs);
	outputsController.registerEndpoints(httpdHandle);
	httpServer.registerFrontendHandler();
	httpServer.registerCorsHandler();
}

/**
 * Initializes MQTT client
 */
void initMqtt() {
	MqttConfig config = MqttConfig();
	MqttLastWillAndTestament lwt = MqttLastWillAndTestament(SbcPduManagement::getDeviceBaseTopic() + "/status", "offline", 2, true);
	config.setLastWillAndTestament(&lwt);
	Mqtt *mqtt = new Mqtt(config);
	mqtt->setOnConnect(mqttConnectCallback);
	homeAssistant = new HomeAssistant(mqtt, &outputs);
	pduManagement = new SbcPduManagement(mqtt, &outputs);
	mqtt->connect();
}

/**
 * Initializes NVS storage
 */
void initNvs() {
	ESP_ERROR_CHECK(NvsManager::init());
	// WiFi NVS
	NvsManager wifiNvs("wifi");
	wifiNvs.setDefault("authmode", static_cast<uint8_t>(WIFI_AUTH_WPA2_PSK));
	wifiNvs.setStringDefault("ssid", "SSID");
	wifiNvs.setStringDefault("psk", "password");
	wifiNvs.commit();
	// MQTT NVS
	NvsManager mqttNvs("mqtt");
	mqttNvs.setStringDefault("uri", "mqtts://mqtt.romanondracek.cz:8883");
	mqttNvs.setStringDefault("username", "sbc_pdu");
	mqttNvs.setStringDefault("password", "password");
	mqttNvs.commit();
	// SNTP NVS
	NvsManager ntpNvs("ntp");
	uint8_t ntpServers = 2;
	ntpNvs.setDefault("servers", ntpServers);
	ntpNvs.setStringDefault("server0", "tik.cesnet.cz");
	ntpNvs.setStringDefault("server1", "tak.cesnet.cz");
	ntpNvs.setStringDefault("timezone", "CET/CEST");
	ntpNvs.commit();
	// HTTP credentials NVS
	NvsManager httpCredentialsNvs("httpCredentials");
	httpCredentialsNvs.setStringDefault("username", "admin");
	httpCredentialsNvs.setStringDefault("password", "sbc-pdu");
	httpCredentialsNvs.commit();
}

/**
 * Initializes outputs
 * @param i2c I2C master driver
 */
void initOutputs(I2C *i2c) {
	Ina3221 *ina3221 = new Ina3221(i2c, INA3221_ADDRESS_GND);
	#if REVISION == 1
		ina3221->writeConfiguration(
			INA3221_MODE_SHUNT_AND_BUS_CONTINUOUS |
			INA3221_SHUNT_CT_8244 |
			INA3221_BUS_CT_8244 |
			INA3221_AVG_1024 |
			INA3221_CHANNEL_1_ENABLE |
			INA3221_CHANNEL_2_DISABLE |
			INA3221_CHANNEL_3_ENABLE
		);
		outputs.insert({1, new Output(ina3221, INA3221_CHANNEL_1, GPIO_NUM_32, GPIO_NUM_35, GPIO_NUM_MAX, 1)});
		outputs.insert({2, new Output(ina3221, INA3221_CHANNEL_3, GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_MAX, 2)});
	#elif REVISION == 2
	#elif REVISION == 3
		ina3221->writeConfiguration(
			INA3221_MODE_SHUNT_AND_BUS_CONTINUOUS |
			INA3221_SHUNT_CT_8244 |
			INA3221_BUS_CT_8244 |
			INA3221_AVG_1024 |
			INA3221_CHANNEL_1_ENABLE |
			INA3221_CHANNEL_2_ENABLE |
			INA3221_CHANNEL_3_ENABLE
		);
		outputs.insert({1, new Output(ina3221, INA3221_CHANNEL_3, GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_21, 1)});
		outputs.insert({2, new Output(ina3221, INA3221_CHANNEL_2, GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33, 2)});
		outputs.insert({3, new Output(ina3221, INA3221_CHANNEL_1, GPIO_NUM_32, GPIO_NUM_35, GPIO_NUM_34, 3)});
	#endif
	for (const auto& outputPair : outputs) {
		ESP_ERROR_CHECK(outputPair.second->addAlertHandler(gpioAlertHandler));
	}
	alertQueue = xQueueCreate(10, sizeof(uint32_t));
	xTaskCreate(alertTask, "alertTask", 4096, nullptr, 10, nullptr);
}

/**
 * Main function
 */
void app_main() {
	// Initialize TCP/IP network interface
	ESP_ERROR_CHECK(esp_netif_init());
	// Create default event loop that running in background
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
	initNvs();
	// Install GPIO ISR service
	gpio_install_isr_service(0);
	I2C *i2c = new I2C(I2C_NUM_0, GPIO_NUM_4, GPIO_NUM_5);
	i2c->scan();
	Mcp7940n *rtc = new Mcp7940n(i2c);
	rtc->enableOscillator();
	initOutputs(i2c);
	HostnameManager *hostname = new HostnameManager();
	#if REVISION == 2
		Ethernet ethernet = Ethernet(hostname);
	#endif
	Wifi *wifi = new Wifi(hostname);
	MulticastDns mDns = MulticastDns(hostname);
	Ntp ntp = Ntp(rtc);
	initHttp(wifi, hostname);
	initMqtt();
	while (1) {
		float totalCurrent = 0;
		std::pair<float, Output*> maxCurrent = {0, nullptr};
		for (const auto& outputPair : outputs) {
			Output *output = outputPair.second;
			float current = fabs(output->readCurrent());
			totalCurrent += current;
			if (current > maxCurrent.first) {
				maxCurrent = {current, output};
			}
			if (pduManagement != nullptr) {
				pduManagement->publishOutputMeasurements(output);
			}
		}
		if (totalCurrent > 4200) {
			uint32_t index = maxCurrent.second->getIndex();
			ESP_LOGE("Output", "Total current %f mA is too high, disabling output %lu", totalCurrent, index);
			maxCurrent.second->enable(false);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
