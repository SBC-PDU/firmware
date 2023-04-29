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

#include <time.h>

#include <esp_log.h>

#include "i2c_master.h"
#include "utils/bcd.h"

enum mcp7940n_registers {
	/// Timekeeping second and oscillator enable bit register
	MCP7940N_REG_RTCSEC = 0x00,
	/// Timekeeping minute register
	MCP7940N_REG_RTCMIN = 0x01,
	/// Timekeeping hour register
	MCP7940N_REG_RTCHOUR = 0x02,
	/// Timekeeping weekday register
	MCP7940N_REG_RTCWKDAY = 0x03,
	/// Timekeeping day register
	MCP7940N_REG_RTCDATE = 0x04,
	/// Timekeeping month register
	MCP7940N_REG_RTCMTH = 0x05,
	/// Timekeeping year register
	MCP7940N_REG_RTCYEAR = 0x06,
	/// Control register
	MCP7940N_REG_CONTROL = 0x07,
	/// Oscillator trim register
	MCP7940N_REG_OSCTRIM = 0x08,
};

enum mcp7940n_bits {
	/// RTC start oscillator bit
	MCP7940N_BIT_ST = 0x80,
	/// RTC external battery backup enable bit
	MCP7940N_BIT_VBATEN = 0x08,
	/// RTC oscillator running bit
	MCP7940N_BIT_OSCRUN = 0x20,
};

/**
 * Microchip MCP7940N RTC driver
 */
class Mcp7940n {
	public:
		/**
		 * Constructor
		 * @param i2c I2C master driver
		 */
		explicit Mcp7940n(I2C *i2c);

		/**
		 * Checks if the RTC oscillator is running
		 * @return true RTC oscillator is running
		 * @return false RTC oscillator is not running
		 */
		bool isOscillatorRunning();

		/**
		 * Enables the RTC oscillator
		 * @return Execution status
		 */
		esp_err_t enableOscillator();

		/**
		 * Disables the RTC oscillator
		 * @return Execution status
		 */
		esp_err_t disableOscillator();

		/**
		 * Returns the RTC time
		 * @param time RTC time
		 * @return Execution status
		 */
		esp_err_t getTime(tm * time);

		/**
		 * Sets the RTC time
		 * @param time Time to set
		 * @return Execution status
		 */
		esp_err_t setTime(tm *time);
	private:
		/// I2C address
		static constexpr uint8_t MCP7940N_ADDRESS = 0x6F;
		/// Logger tag
		static constexpr const char *TAG = "MCP7940N RTC";
		/// I2C master driver
		I2C *i2c;
};
