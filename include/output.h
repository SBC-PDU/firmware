#pragma once

#include <driver/gpio.h>

#include "ina3221.h"

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
         */
        Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert);

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
         * @param args Paramameter for interrupt handler
         * @return esp_err_t 
         */
        esp_err_t addAlertHandler(gpio_isr_t handler, void* args);

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
};