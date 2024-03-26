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
#include "i2c_master.h"

I2C::I2C(i2c_port_t port, gpio_num_t sda, gpio_num_t scl): port(port) {
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

void I2C::scan() {
	static const char* TAG = "I2C";
	ESP_LOGI(TAG, "Starting I2C scan...");
	for (uint8_t address = 1; address < 127; ++address) {
		// Creates and initializes I2C command link
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		// Sends device address
		i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
		i2c_master_stop(cmd);
		esp_err_t result = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		if (result == ESP_OK) {
			ESP_LOGI(TAG, "Found device at: 0x%2x", address);
		}
	}
	ESP_LOGI(TAG, "Scan completed.");
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
