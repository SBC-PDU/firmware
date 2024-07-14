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
#include "mcp7940n.h"

Mcp7940n::Mcp7940n(I2C *i2c): i2c(i2c) {
}

esp_err_t Mcp7940n::enableOscillator() {
	uint8_t buffer[1] = {0,};
	esp_err_t ret = this->i2c->read(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 1);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading RTCSEC register: %d (%s)", ret, esp_err_to_name(ret));
		return ret;
	}
	vTaskDelay(50 / portTICK_PERIOD_MS);
	buffer[0] |= MCP7940N_BIT_ST;
	ret = this->i2c->write(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 1);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error enabling oscillator: %d (%s)", ret, esp_err_to_name(ret));
	}
	vTaskDelay(50 / portTICK_PERIOD_MS);
	return ret;
}

esp_err_t Mcp7940n::disableOscillator() {
	uint8_t buffer[1] = {0,};
	esp_err_t ret = this->i2c->read(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 1);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading RTCSEC register: %d (%s)", ret, esp_err_to_name(ret));
		return ret;
	}
	vTaskDelay(50 / portTICK_PERIOD_MS);
	buffer[0] &= ~(MCP7940N_BIT_ST);
	ret = this->i2c->write(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 1);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error disabling oscillator: %d (%s)", ret, esp_err_to_name(ret));
	}
	vTaskDelay(50 / portTICK_PERIOD_MS);
	return ret;
}

bool Mcp7940n::isOscillatorRunning() {
	uint8_t buffer[1] = {0,};
	esp_err_t ret = this->i2c->read(MCP7940N_ADDRESS, MCP7940N_REG_RTCWKDAY, buffer, 1);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading oscillator status: %d (%s)", ret, esp_err_to_name(ret));
	}
	return (buffer[0] & MCP7940N_BIT_OSCRUN) != 0;
}

esp_err_t Mcp7940n::getTime(tm *time) {
	uint8_t buffer[7] = {0,};
	esp_err_t ret = this->i2c->read(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 7);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading time: %d (%s)", ret, esp_err_to_name(ret));
		return ret;
	}
	if (time == nullptr) {
		ESP_LOGE(TAG, "Time structure is null");
		return ESP_ERR_INVALID_ARG;
	}
	time->tm_sec = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCSEC] & 0x7F);
	time->tm_min = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCMIN] & 0x7F);
	time->tm_hour = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCHOUR] & 0x3F);
	time->tm_wday = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCWKDAY] & 0x07) - 1;
	time->tm_mday = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCDATE] & 0x3F);
	time->tm_mon = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCMTH] & 0x1F) - 1;
	time->tm_year = Bcd::bcd2bin(buffer[MCP7940N_REG_RTCYEAR]) + 100;
	char strBuffer[64];
	strftime(strBuffer, sizeof(strBuffer), "%c", time);
	ESP_LOGI(TAG, "Date/time  is: %s", strBuffer);
	return ret;
}

esp_err_t Mcp7940n::setTime(tm *time) {
	uint8_t buffer[7] = {0,};
	if (time == nullptr) {
		ESP_LOGE(TAG, "Time structure is null");
		return ESP_ERR_INVALID_ARG;
	}
	char strBuffer[64];
	strftime(strBuffer, sizeof(strBuffer), "%c", time);
	ESP_LOGI(TAG, "Date/time to set is: %s", strBuffer);
	buffer[MCP7940N_REG_RTCSEC] = Bcd::bin2bcd(time->tm_sec) | MCP7940N_BIT_ST;
	buffer[MCP7940N_REG_RTCMIN] = Bcd::bin2bcd(time->tm_min);
	buffer[MCP7940N_REG_RTCHOUR] = Bcd::bin2bcd(time->tm_hour);
	buffer[MCP7940N_REG_RTCWKDAY] = Bcd::bin2bcd(time->tm_wday + 1) | MCP7940N_BIT_VBATEN;
	buffer[MCP7940N_REG_RTCDATE] = Bcd::bin2bcd(time->tm_mday);
	buffer[MCP7940N_REG_RTCMTH] = Bcd::bin2bcd(time->tm_mon + 1);
	buffer[MCP7940N_REG_RTCYEAR] = Bcd::bin2bcd(time->tm_year - 100);
	esp_err_t ret = this->i2c->write(MCP7940N_ADDRESS, MCP7940N_REG_RTCSEC, buffer, 7);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error setting time: %d (%s)", ret, esp_err_to_name(ret));
	}
	return ret;
}
