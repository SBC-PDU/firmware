#pragma once

#include <driver/gpio.h>

#include "ina3221.h"
#include "network/mqtt.h"

/**
 * @brief USB output
 */
class Output {
    public:
        /**
         * @brief Construct a new Output object
         * 
         * @param ina3221 Pointer to INA3221 driver instance
         * @param channel INA3221 channel ID
         * @param enable Enable GPIO pin
         * @param alert Alert GPIO pin
         * @param index Output index
         */
        Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert, uint8_t index);

        /**
         * @brief Is the output in alert state?
         * 
         * @return true Alert occured - over-current or over-temperature protection asserted
         * @return false Normal operation
         */
        bool hasAlert();

        /**
         * @brief Adds the alert interrupt handler
         * 
         * @param handler Interrupr handler
         * @return esp_err_t 
         */
        esp_err_t addAlertHandler(gpio_isr_t handler);

        /**
         * @brief Removes the alert interrupr handler
         * 
         * @return esp_err_t 
         */
        esp_err_t removeAlertHandler();

        /**
         * @brief Enables the output
         * 
         * @param enabled Output enablement
         */
        void enable(bool enabled);

        /**
         * @brief Is the output enabled?
         * 
         * @return true Output enabled
         * @return false Output disabled
         */
        bool isEnabled();

        /**
         * @brief Reads the current flowing through the output
         * 
         * @return float Current in milliamps
         */
        float readCurrent();

        /**
         * @brief Reads voltage on the output
         * 
         * @return float Voltage in volts
         */
        float readVoltage();

        /**
         * @brief Returns the base MQTT topic
         * 
         * @return std::string Base MQTT topic
         */
        std::string getBaseMqttTopic();

        /**
         * @brief Publishes alest state to MQTT
         * 
         * @param mqtt Pointer to MQTT client instance
         */
        void publishAlert(Mqtt *mqtt);

        /**
         * @brief Publishes measurements to MQTT 
         * 
         * @param mqtt Pointer to MQTT client instance
         */
        void publishMeasurements(Mqtt *mqtt);

        /**
         * @brief MQTT receive message callback
         * 
         * @param event MQTT event
         */
        void receiveMqttCallback(esp_mqtt_event_handle_t event);

        /**
         * @brief Subscribes for output enablement state
         * 
         * @param mqtt Pointer to MQTT client instance
         * @param callback Subscribe callback
         */
        void subscribeEnablement(Mqtt *mqtt, Mqtt::subscribe_callback_t callback);
    public:
        /// @brief Pointer to INA3221 driver instance
        Ina3221 *ina3221;
        /// @brief INA3221 channel ID
        ina3221_channel_t channel;
        /// @brief Output enablement state
        bool enabled = false;
        /// @brief Alert GPIO pin
        gpio_num_t alertPin;
        /// @brief Enable GPIO pin
        gpio_num_t enablePin;
        /// @brief Output index
        uint8_t index;
        /// @brief Base MQTT topic
        std::string baseMqttTopic;
};