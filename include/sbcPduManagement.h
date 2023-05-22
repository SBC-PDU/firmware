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

#include "network/mqtt.h"
#include "network/wifi.h"
#include "output.h"

/**
 * SBC PDU Management client
 */
class SbcPduManagement {
	public:
		/**
		 * Constructor
		 * @param outputs Output map <index, pointer to output>
		 */
		explicit SbcPduManagement(Mqtt *mqtt, std::map<uint8_t, Output*> *outputs);

		/**
		 * MQTT connect callback
		 * @param client MQTT client
		 * @param base MQTT event base
		 * @param event MQTT event
		 */
		static void connectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event);

		/**
		 * Output enablement MQTT message callback
		 * @param event MQTT event
		 */
		static void enablementCallback(esp_mqtt_event_handle_t event);

		/**
		 * Returns the base output MQTT topic
		 * @param output Pointer to the output
		 * @return std::string Base ouput MQTT topic
		 */
		static std::string getOutputBaseTopic(Output *output);

		static std::string getDeviceBaseTopic();

		/**
		 * Publishes output alert state to MQTT
		 * @param output Pointer to the output
		 */
		static void publishOutputAlert(Output *output);

		/**
		 * @brief Publishes output measurements to MQTT
		 * @param output Pointer to the output
		 */
		static void publishOutputMeasurements(Output *output);

		/**
		 * Subscribes to output enablemenr
		 * @param output Output to subscribe
		 */
		static void subscribeOutputEnablement(Output *output);
	protected:
		/// Base MQTT topic
		static std::string baseTopic;
		/// Output map <index, pointer to output>
		static std::map<uint8_t, Output*> *outputs;
	private:
		/// Pointer to the MQTT client
		static Mqtt *mqtt;
		/// Logger tag
		constexpr static const char *TAG = "SbcPduManagement";

};
