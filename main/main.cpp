#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <vector>
#include <cJSON.h>

#include "i2c_master.h"
#include "ina3221.h"
#include "network/hostname.h"
#include "network/http.h"
#include "network/mdns.h"
#include "network/mqtt.h"
#include "network/sntp.h"
#include "network/wifi.h"
#include "nvs_manager.h"
#include "restApi/wifiController.h"
#include "output.h"
#include "spiffs.h"

extern "C" void app_main();

/// @brief Alert queue
static QueueHandle_t alertQueue = nullptr;
/// @brief Pointer to MQTT client instance 
Mqtt *mqtt = nullptr;
/// @brief Pointer to the first PDU output
Output *output1 = nullptr;
/// @brief Pointer to the second PDU output
Output *output2 = nullptr;

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
    Output *output = nullptr;
    if (topic == output1->getBaseMqttTopic() + "/enable") {
        output = output1;
    } else if (topic == output2->getBaseMqttTopic() + "/enable") {
        output = output2;
    }
    output->enable(data == "1");
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
            if (outputId == 1) {
                output1->publishAlert(mqtt);
            } else if (outputId == 2) {
                output2->publishAlert(mqtt);
            }
        }
    }
}


/**
 * @brief Initializes HTTP server
 */
void initHttp(Wifi *wifi) {
    std::string basePath = "/spiffs";
    SPIFFS mainSpiffs(basePath);
    HttpServer httpServer(basePath);
    httpd_handle_t httpdHandle = httpServer.getHandle();
    namespace restApi = sbc_pdu::restApi;
    restApi::WifiController wifiInfo = restApi::WifiController(wifi);
    wifiInfo.registerEndpoints(httpdHandle);
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
}

/**
 * @brief Initializes outputs
 */
void initOutputs() {
    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(output1->addAlertHandler(gpioAlertHandler));
    ESP_ERROR_CHECK(output2->addAlertHandler(gpioAlertHandler));
    alertQueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(alertTask, "alertTask", 4096, nullptr, 10, nullptr);
    output1->subscribeEnablement(mqtt, mqttReceiveCallback);
    output2->subscribeEnablement(mqtt, mqttReceiveCallback);
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
    ESP_ERROR_CHECK(NvsManager::init());
    I2C *i2c = new I2C(I2C_NUM_0, GPIO_NUM_4, GPIO_NUM_5);
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
    output1 = new Output(ina3221, INA3221_CHANNEL_1, GPIO_NUM_32, GPIO_NUM_35, 1);
    output2 = new Output(ina3221, INA3221_CHANNEL_3, GPIO_NUM_33, GPIO_NUM_34, 2);
    NvsManager wifiNvs("wifi");
    wifiNvs.set("authmode", static_cast<uint8_t>(WIFI_AUTH_WPA2_PSK));
    wifiNvs.setString("ssid", "SSID");
    wifiNvs.setString("psk", "password");
    NvsManager mqttNvs("mqtt");
    mqttNvs.setString("uri", "mqtt://mqtt.romanondracek.cz:1883");
    mqttNvs.setString("username", "sbc_pdu");
    mqttNvs.setString("password", "password");
    NvsManager ntpNvs("ntp");
    ntpNvs.setString("server0", "tik.cesnet.cz");
    ntpNvs.setString("server1", "tak.cesnet.cz");
    ntpNvs.setString("timezone", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
    NvsManager httpCredentialsNvs("httpCredentials");
    httpCredentialsNvs.setString("username", "admin");
    httpCredentialsNvs.setString("password", "sbc-pdu");
    HostnameManager *hostname = new HostnameManager();
    Wifi *wifi = new Wifi(hostname);
    MulticastDns mDns = MulticastDns(hostname);
    Ntp();
    initHttp(wifi);
    initMqtt();
    initOutputs();
    while (1) {
        output1->publishMeasurements(mqtt);
        output2->publishMeasurements(mqtt);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

