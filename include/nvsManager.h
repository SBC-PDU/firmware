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

#include <string>

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <nvs_handle.hpp>


/**
 * Logger tag
 */
static const char *LOG_TAG = "NVS";

/**
 * Non-volatile storage manager
 */
class NvsManager {
	public:
		/**
		 * Initializes Non-volatile storage
		 * @return Execution status
		 */
		static esp_err_t init();

		/**
		 * Constructor
		 * @param nameSpace NVS namespace
		 */
		explicit NvsManager(const std::string &nameSpace);

		/**
		 * Commits changes into NVS
		 * @return Execution status
		 */
		esp_err_t commit();

		/**
		 * Removess given key
		 * @param key Key to remove
		 * @return Execution status
		 */
		esp_err_t remove(const std::string &key);

		/**
		 * Obtains string from given key
		 * @param key Key
		 * @param value Obtained value
		 * @return Execution status
		 */
		esp_err_t getString(const std::string &key, std::string &value);

		/**
		 * Obtains unsigned 8-bit integer from given key
		 * @param key Key
		 * @param value Obtained value
		 * @return Execution status
		 */
		template<typename T>
		esp_err_t get(const std::string &key, T &value) {
			ESP_LOGI(LOG_TAG, "Reading \"%s\"...", key.c_str());
			esp_err_t result = this->handle->get_item(key.c_str(), value);
			if (result != ESP_OK) {
				ESP_LOGE(LOG_TAG, "Failed to obtain %s stored in key \"%s\". Error: %s", typeid(T).name(), key.c_str(), esp_err_to_name(result));
				return result;
			}
			return result;
		}

		/**
		 * Sets string for given key
		 * @param key Key
		 * @param value Value to set
		 * @return Execution status
		 */
		esp_err_t setString(const std::string &key, const std::string &value);

		/**
		 * Sets default string for given key
		 * @param key Key
		 * @param value Value to set
		 * @return Execution status
		 */
		esp_err_t setStringDefault(const std::string &key, const std::string &value);

		/**
		 * Sets numeric value for given key
		 * @param key Key
		 * @param value Value to set
		 * @return Execution status
		 */
		template<typename T>
		esp_err_t set(const std::string &key, const T &value) {
			ESP_LOGI(LOG_TAG, "Writing %s \"%d\" to key \"%s\".", typeid(T).name(), value, key.c_str());
			esp_err_t result = this->handle->set_item(key.c_str(), value);
			if (result != ESP_OK) {
				ESP_LOGE(LOG_TAG, "Failed to set %s \"%d\" to key \"%s\". Error: %s", typeid(T).name(), value, key.c_str(), esp_err_to_name(result));
			}
			return result;
		}

		/**
		 * Sets default numeric value for given key
		 * @param key Key
		 * @param value Value to set
		 * @return Execution status
		 */
		template<typename T>
		esp_err_t setDefault(const std::string &key, const T &value) {
			ESP_LOGI(LOG_TAG, "Writing default %s \"%d\" to key \"%s\".", typeid(T).name(), value, key.c_str());
			T currentValue;
			esp_err_t result = this->handle->get_item(key.c_str(), currentValue);
			if (result == ESP_ERR_NVS_NOT_FOUND) {
				result = this->handle->set_item(key.c_str(), value);
			}
			if (result == ESP_OK) {
				ESP_LOGI(LOG_TAG, "Key \"%s\" (%s) is alreasy set.", key.c_str(), typeid(T).name());
			} else {
				ESP_LOGE(LOG_TAG, "Failed to set default %s \"%d\" to key \"%s\". Error: %s", typeid(T).name(), value, key.c_str(), esp_err_to_name(result));
			}
			return result;
		}

	private:
		/// NVS handle
		std::shared_ptr<nvs::NVSHandle> handle;

};
