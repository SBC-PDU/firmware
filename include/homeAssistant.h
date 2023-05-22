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

#include <map>
#include <string>
#include <vector>

#include <esp_ota_ops.h>
#include <cJSON.h>

#include "network/mqtt.h"
#include "network/wifi.h"
#include "output.h"
#include "sbcPduManagement.h"

typedef struct HomeAssistantSensor {
	std::string name;
	std::string icon;
	std::string deviceClass;
	std::string unitOfMeasurement;
} haSensor_t;

/**
 * Home Assistant MQTT client
 */
class HomeAssistant {
	public:
		/**
		 * Constructor
		 * @param outputs Output map <index, pointer to output>
		 */
		explicit HomeAssistant(Mqtt *mqtt, std::map<uint8_t, Output*> *outputs);

		/**
		 * MQTT connect callback
		 * @param client MQTT client
		 * @param base MQTT event base
		 * @param event MQTT event
		 */
		static void connectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event);

		static void advertise(Mqtt *mqtt);

		static void advertiseAlert(Mqtt *mqtt, Output *output, cJSON *device);

		static void advertiseSensors(Mqtt *mqtt, Output *output, cJSON *device);

		static void advertiseSwitch(Mqtt *mqtt, Output *output, cJSON *device);

	protected:
		/// Base MQTT topic
		static std::string baseTopic;
		/// Output map <index, pointer to output>
		static std::map<uint8_t, Output*> *outputs;
	private:
		/// Pointer to the MQTT client
		Mqtt *mqtt;
		/// Logger tag
		constexpr static const char *TAG = "HomeAssistant";
};
