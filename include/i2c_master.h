#pragma once

#include <driver/gpio.h>
#include <driver/i2c.h>

#include <esp_err.h>
#include <esp_log.h>

/// I2C master will check ACK
#define ACK_CHECK_EN    0x1
/// I2C master will not check ACK
#define ACK_CHECK_DIS   0x0

/**
 * @brief I2C master
 */
class I2C {
    public:
        /**
         * @brief Construct a new I2C master instance
         * 
         * @param port I2C port ID
         * @param sda SDA GPIO pin
         * @param scl SCL GPIO pin
         */
        I2C(int port, gpio_num_t sda, gpio_num_t scl);

        /**
         * @brief Reads data from I2C slave device
         * 
         * @param address Address
         * @param reg Register
         * @param buffer Buffer
         * @param size Buffer size
         * @return esp_err_t Execution status
         */
        esp_err_t read(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size);

        /**
         * @brief Writes data to I2C slave device
         * 
         * @param address Address
         * @param reg Register
         * @param buffer Buffer
         * @param size Buffer size
         * @return esp_err_t Execution status
         */
        esp_err_t write(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size);
    protected:
        /// Logger tag
        static constexpr const char *LOG_TAG = "I2C";
    private:
        /// @brief I2C port ID
        int port;
};
