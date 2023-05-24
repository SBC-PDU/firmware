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
#include "output.h"

QueueHandle_t Output::buttonQueue = nullptr;

Output::Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert, gpio_num_t button, uint8_t index): ina3221(ina3221), channel(channel), alertPin(alert), buttonPin(button), enablePin(enable), index(index) {
	gpio_config_t alertConfig = {
		.pin_bit_mask = (1ULL << static_cast<int>(this->alertPin)),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_ANYEDGE,
	};
	ESP_ERROR_CHECK(gpio_config(&alertConfig));
	gpio_config_t enableConfig = {
		.pin_bit_mask = (1ULL << static_cast<int>(this->enablePin)),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};
	ESP_ERROR_CHECK(gpio_config(&enableConfig));
	ESP_ERROR_CHECK(gpio_set_level(this->enablePin, 1));
	if (button != GPIO_NUM_MAX) {
		gpio_config_t buttonConfig = {
			.pin_bit_mask = (1ULL << static_cast<int>(this->buttonPin)),
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_POSEDGE,
		};
		ESP_ERROR_CHECK(gpio_config(&buttonConfig));
		ESP_ERROR_CHECK(gpio_isr_handler_add(this->buttonPin, Output::buttonHandler, (void *) this));
		if (Output::buttonQueue == nullptr) {
			Output::buttonQueue = xQueueCreate(10, sizeof(Output *));
		}
		xTaskCreate(Output::buttonTask, "buttonTask", 4096, nullptr, 10, nullptr);
	}
}

void IRAM_ATTR Output::buttonHandler(void *arg) {
	Output *output = static_cast<Output *>(arg);
	xQueueSendFromISR(Output::buttonQueue, &output, nullptr);
}

void Output::buttonTask(void *arg) {
	Output *output;
	while (true) {
		if (xQueueReceive(Output::buttonQueue, &output, portMAX_DELAY)) {
			if (output != nullptr) {
				output->enable(!output->isEnabled());
			}
		}
	}
}

bool Output::hasAlert() {
	return gpio_get_level(this->alertPin) == 0;
}

esp_err_t Output::addAlertHandler(gpio_isr_t handler) {
	return gpio_isr_handler_add(this->alertPin, handler, (void *) this->index);
}

esp_err_t Output::removeAlertHandler() {
	return gpio_isr_handler_remove(this->alertPin);
}

void Output::enable(bool enabled) {
	this->enabled = enabled;
	ESP_ERROR_CHECK(gpio_set_level(this->enablePin, static_cast<uint32_t>(!enabled)));
}

bool Output::isEnabled() {
	return this->enabled;
}

float Output::readCurrent() {
	float current = this->ina3221->readCurrent(this->channel, 50);
#if REVISION == 3
	if (current != 0) {
		current = fabs(current - 1.6);
	}
#endif
	return current;
}

float Output::readVoltage() {
	return this->ina3221->readBusVoltage(this->channel);
}

uint32_t Output::getIndex() {
	return this->index;
}
