#include "output.h"

Output::Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert, uint8_t index): ina3221(ina3221), channel(channel), alertPin(alert), enablePin(enable), index(index) {
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
    this->baseMqttTopic = Mqtt::getBaseTopic() + "/outputs/" + std::to_string(this->index);
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
    return this->ina3221->readCurrent(this->channel, 50);
}

float Output::readVoltage() {
    return this->ina3221->readBusVoltage(this->channel);
}

std::string Output::getBaseMqttTopic() {
    return this->baseMqttTopic;
}

void Output::publishAlert(Mqtt *mqtt) {
    mqtt->publishString(this->baseMqttTopic + "/alert", std::to_string(this->hasAlert()), 2, false);
}

void Output::publishMeasurements(Mqtt *mqtt) {
    this->publishAlert(mqtt);
    mqtt->publishString(this->baseMqttTopic + "/enabled", std::to_string(this->isEnabled()), 2, false);
    mqtt->publishString(this->baseMqttTopic + "/current", std::to_string(fabs(this->readCurrent())), 2, false);
    mqtt->publishString(this->baseMqttTopic + "/voltage", std::to_string(this->readVoltage()), 2, false);

}

void Output::subscribeEnablement(Mqtt* mqtt, Mqtt::subscribe_callback_t callback) {
    mqtt->subscribe(this->baseMqttTopic + "/enable", callback, 2);
}
