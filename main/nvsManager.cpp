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
#include "nvsManager.h"

esp_err_t NvsManager::init() {
	esp_err_t result = nvs_flash_init();
	if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		result = nvs_flash_init();
	}
	return result;
}

NvsManager::NvsManager(const std::string &nameSpace) {
	ESP_LOGI(LOG_TAG, "Opening Non-Volatile Storage (NVS) handle (%s)...", nameSpace.c_str());
	esp_err_t result;
	this->handle = nvs::open_nvs_handle(nameSpace.c_str(), NVS_READWRITE, &result);
	if (result != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(result));
		return;
	}
	ESP_LOGI(LOG_TAG, "Done");
}

esp_err_t NvsManager::commit() {
	ESP_LOGI(LOG_TAG, "Committing updates...");
	esp_err_t result = this->handle->commit();
	if (result == ESP_OK) {
		ESP_LOGI(LOG_TAG, "Updates comitted.");
	} else {
		ESP_LOGE(LOG_TAG, "Failed to commit updates. Error: %s", esp_err_to_name(result));
	}
	return result;
}

esp_err_t NvsManager::getString(const std::string &key, std::string &value) {
	ESP_LOGI(LOG_TAG, "Reading \"%s\"...", key.c_str());
	size_t size;
	esp_err_t result = this->handle->get_item_size(nvs::ItemType::SZ, key.c_str(), size);
	if (result != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Failed to obtain string lenght stored in key \"%s\". Error: %s", key.c_str(), esp_err_to_name(result));
		return result;
	}
	char *buffer = new char[size];
	result = this->handle->get_string(key.c_str(), buffer, size);
	if (result != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Failed to obtain string stored in key \"%s\". Error: %s", key.c_str(), esp_err_to_name(result));
		delete[] buffer;
		return result;
	}
	value = std::string(buffer);
	delete[] buffer;
	return result;
}

esp_err_t NvsManager::setString(const std::string &key, const std::string &value) {
	ESP_LOGI(LOG_TAG, "Writing string \"%s\" to key \"%s\".", value.c_str(), key.c_str());
	esp_err_t result = this->handle->set_string(key.c_str(), value.c_str());
	if (result != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Failed to set string \"%s\" to key \"%s\". Error: %s", value.c_str(), key.c_str(), esp_err_to_name(result));
	}
	return result;
}

esp_err_t NvsManager::setStringDefault(const std::string &key, const std::string &value) {
	ESP_LOGI(LOG_TAG, "Writing default string \"%s\" to key \"%s\".", value.c_str(), key.c_str());
	size_t size;
	esp_err_t result = this->handle->get_item_size(nvs::ItemType::SZ, key.c_str(), size);
	if (result == ESP_ERR_NVS_NOT_FOUND) {
		result = this->handle->set_string(key.c_str(), value.c_str());
	}
	if (result == ESP_OK) {
		ESP_LOGI(LOG_TAG, "Key \"%s\" is already set.", key.c_str());
	} else {
		ESP_LOGE(LOG_TAG, "Failed to set default string \"%s\" to key \"%s\". Error: %s", value.c_str(), key.c_str(), esp_err_to_name(result));
	}
	return result;
}

esp_err_t NvsManager::remove(const std::string &key) {
	ESP_LOGI(LOG_TAG, "Removing key \"%s\"...", key.c_str());
	esp_err_t result = this->handle->erase_item(key.c_str());
	if (result != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Failed to remove key \"%s\". Error: %s", key.c_str(), esp_err_to_name(result));
	}
	return result;
}
