#pragma once

#include <driver/gpio.h>
#include <driver/i2c.h>

#include <esp_err.h>
#include <esp_log.h>

/// I2C master will check ACK
#define ACK_CHECK_EN    0x1
/// I2C master will not check ACK
#define ACK_CHECK_DIS   0x0

class I2C {
    public:
        I2C(int port, gpio_num_t sda, gpio_num_t scl);
        esp_err_t read(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size);
        esp_err_t write(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size);
    protected:
        /// Logger tag
        static constexpr const char *LOG_TAG = "I2C";
    private:
        int port;
};
