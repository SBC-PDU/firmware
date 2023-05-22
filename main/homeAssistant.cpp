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
#include "homeAssistant.h"

std::string HomeAssistant::baseTopic = "homeassistant/";

std::map<uint8_t, Output*> *HomeAssistant::outputs = nullptr;

HomeAssistant::HomeAssistant(Mqtt *mqtt, std::map<uint8_t, Output*> *outputs): mqtt(mqtt) {
	this->outputs = outputs;
}

void HomeAssistant::advertiseAlert(Mqtt *mqtt, Output *output, cJSON *device) {
	cJSON *root = cJSON_CreateObject();
	std::string baseUniqueId = "sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_alert";
	std::string topic = HomeAssistant::baseTopic + "binary_sensor/sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_alert/config";
	std::string name = "Output #" + std::to_string(output->getIndex()) + " alert";
	cJSON_AddStringToObject(root, "name", name.c_str());
	cJSON_AddStringToObject(root, "availability_topic", (SbcPduManagement::getDeviceBaseTopic() + "/status").c_str());
	cJSON_AddItemToObject(root, "device", device);
	cJSON_AddStringToObject(root, "device_class", "problem");
	cJSON_AddTrueToObject(root, "enabled_by_default");
	cJSON_AddStringToObject(root, "icon", "mdi:fuse-alert");
	cJSON_AddStringToObject(root, "state_topic", (SbcPduManagement::getOutputBaseTopic(output) + "/alert").c_str());
	cJSON_AddStringToObject(root, "payload_on", "1");
	cJSON_AddStringToObject(root, "payload_off", "0");
	cJSON_AddStringToObject(root, "unique_id", baseUniqueId.c_str());
	const char *response = cJSON_PrintUnformatted(root);
	mqtt->publishString(topic, std::string(response), 2, true);
	delete response;
	cJSON_DetachItemFromObject(root, "device");
	cJSON_Delete(root);
}

void HomeAssistant::advertiseSensors(Mqtt *mqtt, Output *output, cJSON *device) {
	std::vector<haSensor_t> sensors = {
		{
			{
				.name = "current",
				.icon = "mdi:current-dc",
				.deviceClass = "current",
				.unitOfMeasurement = "mA"
			},
			{
				.name = "voltage",
				.icon = "mdi:current-dc",
				.deviceClass = "voltage",
				.unitOfMeasurement = "V"
			}
		},
	};
	for (auto const &sensor : sensors) {
		cJSON *root = cJSON_CreateObject();
		std::string baseUniqueId = "sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_" + sensor.deviceClass;
		std::string topic = HomeAssistant::baseTopic + "sensor/sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_" + sensor.deviceClass + "/config";
		std::string name = "Output #" + std::to_string(output->getIndex()) + " " + sensor.name;
		cJSON_AddStringToObject(root, "name", name.c_str());
		cJSON_AddStringToObject(root, "availability_topic", (SbcPduManagement::getDeviceBaseTopic() + "/status").c_str());
		cJSON_AddItemToObject(root, "device", device);
		cJSON_AddStringToObject(root, "device_class", sensor.deviceClass.c_str());
		cJSON_AddTrueToObject(root, "enabled_by_default");
		cJSON_AddStringToObject(root, "icon", sensor.icon.c_str());
		cJSON_AddStringToObject(root, "state_topic", (SbcPduManagement::getOutputBaseTopic(output) + "/" + sensor.deviceClass).c_str());
		cJSON_AddStringToObject(root, "unique_id", baseUniqueId.c_str());
		cJSON_AddStringToObject(root, "unit_of_measurement", sensor.unitOfMeasurement.c_str());
		const char *response = cJSON_PrintUnformatted(root);
		mqtt->publishString(topic, std::string(response), 2, true);
		delete response;
		cJSON_DetachItemFromObject(root, "device");
		cJSON_Delete(root);
	}
}

void HomeAssistant::advertiseSwitch(Mqtt *mqtt, Output *output, cJSON *device) {
		cJSON *root = cJSON_CreateObject();
		std::string baseUniqueId = "sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_switch";
		std::string topic = HomeAssistant::baseTopic + "switch/sbc-pdu_" + Wifi::getPrimaryMacAddress() + "_output_" + std::to_string(output->getIndex()) + "_switch/config";
		std::string outputTopic = SbcPduManagement::getOutputBaseTopic(output);
		std::string name = "Output #" + std::to_string(output->getIndex());
		cJSON_AddStringToObject(root, "name", name.c_str());
		cJSON_AddStringToObject(root, "availability_topic", (SbcPduManagement::getDeviceBaseTopic() + "/status").c_str());
		cJSON_AddItemToObject(root, "device", device);
		cJSON_AddStringToObject(root, "state_topic", (outputTopic + "/enabled").c_str());
		cJSON_AddStringToObject(root, "command_topic", (outputTopic + "/enable").c_str());
		cJSON_AddStringToObject(root, "payload_on", "1");
		cJSON_AddStringToObject(root, "payload_off", "0");
		cJSON_AddNumberToObject(root, "qos", 2);
		cJSON_AddTrueToObject(root, "enabled_by_default");
		cJSON_AddStringToObject(root, "icon", "mdi:power");
		cJSON_AddStringToObject(root, "unique_id", baseUniqueId.c_str());
		const char *response = cJSON_PrintUnformatted(root);
		mqtt->publishString(topic, std::string(response), 2, true);
		delete response;
		cJSON_DetachItemFromObject(root, "device");
		cJSON_Delete(root);
}

void HomeAssistant::advertise(Mqtt *mqtt) {
	cJSON *device = cJSON_CreateObject();
	cJSON_AddStringToObject(device, "identifiers", Wifi::getPrimaryMacAddress().c_str());
	cJSON_AddStringToObject(device, "name", ("SBC PDU #" + Wifi::getPrimaryMacAddress()).c_str());
	cJSON_AddStringToObject(device, "model", "SBC PDU");
	cJSON_AddStringToObject(device, "manufacturer", "Roman Ondráček");
	const esp_app_desc_t *appDescription = esp_app_get_description();
	cJSON_AddStringToObject(device, "sw_version", appDescription->version);
	cJSON_AddStringToObject(device, "hw_version", std::to_string(REVISION).c_str());
	for (auto const& [index, output] : *HomeAssistant::outputs) {
		HomeAssistant::advertiseAlert(mqtt, output, device);
		HomeAssistant::advertiseSensors(mqtt, output, device);
		HomeAssistant::advertiseSwitch(mqtt, output, device);
	}
	cJSON_Delete(device);
}

void HomeAssistant::connectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event) {
	HomeAssistant::advertise(client);
}
