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
#include "network/multicast_dns.h"
#include "network/sntp.h"
#include "network/wifi.h"
#include "nvs_manager.h"
#include "output.h"
#include "spiffs.h"

extern "C" void app_main();

Output *output1 = nullptr;
Output *output2 = nullptr;
std::string baseTopic = "sbc_pdu/" + Wifi::getPrimaryMacAddress();

void mqttReceiveCallback(esp_mqtt_event_handle_t event) {
    static const char TAG[] = "MQTT";
    ESP_LOGI(TAG, "Received data from MQTT:");
    std::string topic(event->topic, event->topic_len);
    std::string data(event->data, event->data_len);
    Output *output = nullptr;
    if (topic == baseTopic + "/outputs/1/enable") {
        output = output1;
    } else {
        output = output2;
    }
    output->enable(data == "true");
}

void app_main() {
    // Initialize TCP/IP network interface
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
    ESP_ERROR_CHECK(NvsManager::init());
    NvsManager wifiNvs("wifi");
    wifiNvs.set("authmode", static_cast<uint8_t>(3));
    wifiNvs.setString("ssid", "SSID");
    wifiNvs.setString("psk", "0123456789");
    uint8_t authMode = 6;
    wifiNvs.set("authMode", authMode);
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
    Mqtt mqtt = Mqtt(mqttConfig);
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
    mqtt.subscribe(baseTopic + "/outputs/1/enable", mqttReceiveCallback, 2);
    mqtt.subscribe(baseTopic + "/outputs/2/enable", mqttReceiveCallback, 2);
    while (1) {
        mqtt.publishString(baseTopic + "/outputs/1/alert", std::to_string(output1->hasAlert()), 2, false);
        mqtt.publishString(baseTopic + "/outputs/1/current", std::to_string(-output1->readCurrent()), 2, false);
        mqtt.publishString(baseTopic + "/outputs/1/voltage", std::to_string(output1->readVoltage()), 2, false);
        mqtt.publishString(baseTopic + "/outputs/2/alert", std::to_string(output2->hasAlert()), 2, false);
        mqtt.publishString(baseTopic + "/outputs/2/current", std::to_string(output2->readCurrent()), 2, false);
        mqtt.publishString(baseTopic + "/outputs/2/voltage", std::to_string(output2->readVoltage()), 2, false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

