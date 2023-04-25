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

#include "i2c_master.h"
#include "ina3221.h"
#include "network/ethernet.h"
#include "network/hostname.h"
#include "network/http.h"
#include "network/mdns.h"
#include "network/mqtt.h"
#include "network/sntp.h"
#include "network/wifi.h"
#include "nvsManager.h"
#include "restApi/authController.h"
#include "restApi/basicAuthenticator.h"
#include "restApi/hostnameController.h"
#include "restApi/mqttController.h"
#include "restApi/ntpController.h"
#include "restApi/outputsController.h"
#include "restApi/systemController.h"
#include "restApi/wifiController.h"
#include "output.h"
#include "spiffs.h"

extern "C" void app_main();

/// @brief Alert queue
static QueueHandle_t alertQueue = nullptr;
/// @brief Pointer to MQTT client instance 
Mqtt *mqtt = nullptr;
/// @brief Output map <index, pointer to output>
std::map<uint8_t, Output*> outputs = {};

/**
 * @brief MQTT connect callback
 * 
 * @param client MQTT client
 * @param base MQTT event base
 * @param event MQTT event
 */
void mqttConnectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event) {
    mqtt->publishString(Mqtt::getBaseTopic() + "/status", "online", 2, true);
}

/**
 * @brief MQTT receive message callback
 * 
 * @param event NQTT event
 */
void mqttReceiveCallback(esp_mqtt_event_handle_t event) {
    static const char TAG[] = "MQTT";
    ESP_LOGI(TAG, "Received data from MQTT:");
    std::string topic(event->topic, event->topic_len);
    std::string data(event->data, event->data_len);
    for (const auto& outputPair : outputs) {
        if (topic == outputPair.second->getBaseMqttTopic() + "/enable") {
            outputPair.second->enable(data == "1");
        }
    }
}

/**
 * @brief Alert GPIO interrupt handler
 * 
 * @param arg Interrupt argument - PDU output ID
 */
static void IRAM_ATTR gpioAlertHandler(void* arg) {
    uint32_t outputId = (uint32_t) arg;
    xQueueSendFromISR(alertQueue, &outputId, nullptr);
}


/**
 * @brief Alert task - sends MQTT message when alert occurres
 * 
 * @param arg Task argument
 */
static void alertTask(void* arg) {
    uint32_t outputId;
    for (;;) {
        if (xQueueReceive(alertQueue, &outputId, portMAX_DELAY)) {
            auto output = outputs.find(static_cast<uint8_t>(outputId));
            if (output != outputs.end()) {
                output->second->publishAlert(mqtt);
            }
        }
    }
}


/**
 * @brief Initializes HTTP server
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
}

/**
 * @brief Initializes MQTT client
 */
void initMqtt() {
    MqttConfig mqttConfig = MqttConfig();
    MqttLastWillAndTestament mqttLwt = MqttLastWillAndTestament(Mqtt::getBaseTopic() + "/status", "offline", 2, true);
    mqttConfig.setLastWillAndTestament(&mqttLwt);
    mqtt = new Mqtt(mqttConfig);
    mqtt->setOnConnect(mqttConnectCallback);
    mqtt->connect();
    for (const auto& outputPair : outputs) {
        outputPair.second->subscribeEnablement(mqtt, mqttReceiveCallback);
    }
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
 * @brief Initializes outputs
 */
void initOutputs() {
    gpio_install_isr_service(0);
    I2C *i2c = new I2C(I2C_NUM_0, GPIO_NUM_4, GPIO_NUM_5);
    i2c->scan();
    Ina3221 *ina3221 = new Ina3221(i2c, INA3221_ADDRESS_GND);
    ina3221->writeConfiguration(
        INA3221_MODE_SHUNT_AND_BUS_CONTINUOUS |
        INA3221_SHUNT_CT_8244 |
        INA3221_BUS_CT_8244 |
        INA3221_AVG_1024 |
        INA3221_CHANNEL_1_ENABLE |
        INA3221_CHANNEL_2_DISABLE |
        INA3221_CHANNEL_3_ENABLE
    );
    outputs.insert({1, new Output(ina3221, INA3221_CHANNEL_1, GPIO_NUM_32, GPIO_NUM_35, 1)});
    outputs.insert({2, new Output(ina3221, INA3221_CHANNEL_3, GPIO_NUM_33, GPIO_NUM_34, 2)});
    for (const auto& outputPair : outputs) {
        ESP_ERROR_CHECK(outputPair.second->addAlertHandler(gpioAlertHandler));
    }
    alertQueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(alertTask, "alertTask", 4096, nullptr, 10, nullptr);
}

/**
 * @brief Main function
 */
void app_main() {
    // Initialize TCP/IP network interface
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
    initNvs();
    initOutputs();
    HostnameManager *hostname = new HostnameManager();
    //Ethernet ethernet = Ethernet(hostname);
    Wifi *wifi = new Wifi(hostname);
    MulticastDns mDns = MulticastDns(hostname);
    Ntp();
    initHttp(wifi, hostname);
    initMqtt();
    while (1) {
        for (const auto& outputPair : outputs) {
            outputPair.second->publishMeasurements(mqtt);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

