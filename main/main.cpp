#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_tls.h>
#include <vector>

#include "i2c_master.h"
#include "ina3221.h"
#include "network/hostname.h"
#include "network/mqtt.h"
#include "network/sntp.h"
#include "network/wifi.h"
#include "nvs_manager.h"
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
/// @brief Base MQTT topic
std::string baseTopic = "sbc_pdu/" + Wifi::getPrimaryMacAddress();

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
    if (topic == baseTopic + "/outputs/1/enable") {
        output = output1;
    } else if (topic == baseTopic + "/outputs/2/enable") {
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
                mqtt->publishString(baseTopic + "/outputs/1/alert", std::to_string(output1->hasAlert()), 2, false);
            } else if (outputId == 2) {
                mqtt->publishString(baseTopic + "/outputs/2/alert", std::to_string(output2->hasAlert()), 2, false);
            }
        }
    }
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
    NvsManager wifiNvs("wifi");
    wifiNvs.set("authmode", static_cast<uint8_t>(WIFI_AUTH_WPA2_WPA3_PSK));
    wifiNvs.setString("ssid", "SSID");
    wifiNvs.setString("psk", "0123456789");
    NvsManager mqttNvs("mqtt");
    mqttNvs.setString("uri", "mqtt://mqtt.romanondracek.cz:1883");
    mqttNvs.setString("username", "sbc_pdu");
    mqttNvs.setString("password", "password");
    NvsManager ntpNvs("ntp");
    ntpNvs.setString("server0", "tik.cesnet.cz");
    ntpNvs.setString("server1", "tak.cesnet.cz");
    ntpNvs.setString("timezone", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
    HostnameManager *hostname = new HostnameManager();
    Wifi *wifi = new Wifi(hostname);
    Ntp();
    MqttConfig mqttConfig = MqttConfig();
    mqtt = new Mqtt(mqttConfig);
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
    output1 = new Output(ina3221, INA3221_CHANNEL_1, GPIO_NUM_32, GPIO_NUM_35);
    output2 = new Output(ina3221, INA3221_CHANNEL_3, GPIO_NUM_33, GPIO_NUM_34);
    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(output1->addAlertHandler(gpioAlertHandler, (void *) 1));
    ESP_ERROR_CHECK(output2->addAlertHandler(gpioAlertHandler, (void *) 2));
    alertQueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(alertTask, "alertTask", 4096, nullptr, 10, nullptr);
    mqtt->subscribe(baseTopic + "/outputs/1/enable", mqttReceiveCallback, 2);
    mqtt->subscribe(baseTopic + "/outputs/2/enable", mqttReceiveCallback, 2);
    while (1) {
        mqtt->publishString(baseTopic + "/outputs/1/alert", std::to_string(output1->hasAlert()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/1/enabled", std::to_string(output1->isEnabled()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/1/current", std::to_string(-output1->readCurrent()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/1/voltage", std::to_string(output1->readVoltage()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/2/alert", std::to_string(output2->hasAlert()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/2/enabled", std::to_string(output2->isEnabled()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/2/current", std::to_string(output2->readCurrent()), 2, false);
        mqtt->publishString(baseTopic + "/outputs/2/voltage", std::to_string(output2->readVoltage()), 2, false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

