#include "i2c_master.h"

I2C::I2C(int port, gpio_num_t sda, gpio_num_t scl): port(port) {
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master = {.clk_speed = 100000},
        .clk_flags = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(this->port, &config));
    ESP_ERROR_CHECK(i2c_driver_install(this->port, config.mode, 0, 0, 0));
}

esp_err_t I2C::read(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size) {
    if (size == 0) {
        return ESP_OK;
    }
    // Creates and initializes I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Sends device address
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    // Sends register we want
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    // Sends device address
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    // Read data
    if (size > 1) {
        i2c_master_read(cmd, buffer, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buffer + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(this->port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t I2C::write(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size) {
    // Creates and initializes I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Sends device address
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    // Sends register we want
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    // Writes the data
    i2c_master_write(cmd, buffer, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(this->port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Could not write to device: %d", ret);
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}
