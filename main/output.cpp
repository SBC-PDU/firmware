#include "output.h"

Output::Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert): ina3221(ina3221), channel(channel), alertPin(alert), enablePin(enable) {
    gpio_config_t alertConfig = {
        .pin_bit_mask = (1ULL << static_cast<int>(this->alertPin)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
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
}

bool Output::hasAlert() {
    return gpio_get_level(this->alertPin) == 0;
}

void Output::enable(bool enabled) {
    ESP_ERROR_CHECK(gpio_set_level(this->enablePin, static_cast<uint32_t>(!enabled)));
}


float Output::readCurrent() {
    return this->ina3221->readCurrent(this->channel, 50);
}

float Output::readVoltage() {
    return this->ina3221->readBusVoltage(this->channel);
}
