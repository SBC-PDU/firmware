/**
 * Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>
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
#include <driver/i2c.h>

#include <esp_err.h>
#include <esp_log.h>

/// I2C master will check ACK
#define ACK_CHECK_EN    0x1
/// I2C master will not check ACK
#define ACK_CHECK_DIS   0x0

/**
 * I2C master
 */
class I2C {
	public:
		/**
		 * Construct a new I2C master instance
		 * @param port I2C port ID
		 * @param sda SDA GPIO pin
		 * @param scl SCL GPIO pin
		 */
		I2C(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

		/**
		 * Scans I2C slaves on the bus
		 */
		void scan();

		/**
		 * Reads data from I2C slave device
		 * @param address Address
		 * @param reg Register
		 * @param buffer Buffer
		 * @param size Buffer size
		 * @return esp_err_t Execution status
		 */
		esp_err_t read(uint8_t address, uint8_t reg, uint8_t *buffer, size_t size);

		/**
		 * Writes data to I2C slave device
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
		/// I2C port ID
		i2c_port_t port;
};
