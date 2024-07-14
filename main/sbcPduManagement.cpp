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
#include "sbcPduManagement.h"

std::string SbcPduManagement::baseTopic = "sbc_pdu/" + Wifi::getPrimaryMacAddress();
Mqtt *SbcPduManagement::mqtt = nullptr;
std::map<uint8_t, Output*> *SbcPduManagement::outputs = nullptr;

SbcPduManagement::SbcPduManagement(Mqtt *mqtt, std::map<uint8_t, Output*> *outputs) {
	this->outputs = outputs;
	this->mqtt = mqtt;
}

void SbcPduManagement::connectCallback(Mqtt* client, esp_event_base_t base, esp_mqtt_event_handle_t event) {
	client->publishString(SbcPduManagement::getDeviceBaseTopic() + "/status", "online", 2, true);
	for (const auto& outputPair : *outputs) {
		SbcPduManagement::subscribeOutputEnablement(outputPair.second);
	}
}

std::string SbcPduManagement::getDeviceBaseTopic() {
	return SbcPduManagement::baseTopic;
}

std::string SbcPduManagement::getOutputBaseTopic(Output *output) {
	return SbcPduManagement::baseTopic + "/outputs/" + std::to_string(output->getIndex());
}

void SbcPduManagement::publishOutputAlert(Output *output) {
	SbcPduManagement::mqtt->publishString(SbcPduManagement::getOutputBaseTopic(output) + "/alert", std::to_string(output->hasAlert()), 2, false);
}

void SbcPduManagement::publishOutputMeasurements(Output *output) {
	std::string topic = SbcPduManagement::getOutputBaseTopic(output);
	SbcPduManagement::publishOutputAlert(output);
	SbcPduManagement::mqtt->publishString(topic + "/enabled", std::to_string(output->isEnabled()), 2, false);
	SbcPduManagement::mqtt->publishString(topic + "/current", std::to_string(fabs(output->readCurrent())), 2, false);
	SbcPduManagement::mqtt->publishString(topic + "/voltage", std::to_string(output->readVoltage()), 2, false);
}

void SbcPduManagement::subscribeOutputEnablement(Output *output) {
	std::string topic = SbcPduManagement::getOutputBaseTopic(output) + "/enable";
	SbcPduManagement::mqtt->subscribe(topic, SbcPduManagement::enablementCallback, 2);
}

void SbcPduManagement::enablementCallback(esp_mqtt_event_handle_t event) {
	std::string topic(event->topic, event->topic_len);
	std::string data(event->data, event->data_len);
	std::string outputTopic;
	for (const auto& outputPair : *outputs) {
		outputTopic = SbcPduManagement::getOutputBaseTopic(outputPair.second) + "/enable";
		if (topic == outputTopic) {
			outputPair.second->enable(data == "1");
		}
	}
}
