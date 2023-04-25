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

#include <driver/gpio.h>

#include "ina3221.h"
#include "network/mqtt.h"

/**
 * Power output
 */
class Output {
    public:
        /**
         * Constructor
         * @param ina3221 Pointer to INA3221 driver instance
         * @param channel INA3221 channel ID
         * @param enable Enable GPIO pin
         * @param alert Alert GPIO pin
         * @param index Output index
         */
        Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert, uint8_t index);

        /**
         * Is the output in alert state?
         * @return true Alert occured - over-current or over-temperature protection asserted
         * @return false Normal operation
         */
        bool hasAlert();

        /**
         * Adds the alert interrupt handler
         * @param handler Interrupr handler
         * @return Execution status
         */
        esp_err_t addAlertHandler(gpio_isr_t handler);

        /**
         * Removes the alert interrupr handler
         * @return Execution status
         */
        esp_err_t removeAlertHandler();

        /**
         * Enables the output
         * @param enabled Output enablement
         */
        void enable(bool enabled);

        /**
         * Is the output enabled?
         * @return true Output enabled
         * @return false Output disabled
         */
        bool isEnabled();

        /**
         * Reads the current flowing through the output
         * @return float Current in milliamps
         */
        float readCurrent();

        /**
         * Reads voltage on the output
         * @return float Voltage in volts
         */
        float readVoltage();

        /**
         * Returns the base MQTT topic
         * @return std::string Base MQTT topic
         */
        std::string getBaseMqttTopic();

        /**
         * Publishes alest state to MQTT
         * @param mqtt Pointer to MQTT client instance
         */
        void publishAlert(Mqtt *mqtt);

        /**
         * Publishes measurements to MQTT 
         * @param mqtt Pointer to MQTT client instance
         */
        void publishMeasurements(Mqtt *mqtt);

        /**
         * MQTT receive message callback
         * @param event MQTT event
         */
        void receiveMqttCallback(esp_mqtt_event_handle_t event);

        /**
         * Subscribes for output enablement state
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
        uint32_t index;
        /// @brief Base MQTT topic
        std::string baseMqttTopic;
};