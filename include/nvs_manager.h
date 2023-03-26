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

    private:
        /// @brief NVS handle
        std::shared_ptr<nvs::NVSHandle> handle;

};