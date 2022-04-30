#include "ina3221.h"


Ina3221::Ina3221(I2C *i2c, ina3221_address_t address): i2c(i2c), address(address) {
}

void Ina3221::writeConfiguration(uint16_t configuration) {
    uint8_t buffer[2] = {static_cast<uint8_t>(configuration >> 8), static_cast<uint8_t>(configuration & 0xff)};
    ESP_ERROR_CHECK(this->i2c->write(static_cast<uint8_t>(this->address), INA3221_REG_CONFIG, buffer, 2));
}

uint16_t Ina3221::readConfiguration() {
    uint8_t buffer[2] = {0,};
    ESP_ERROR_CHECK(this->i2c->read(static_cast<uint8_t>(this->address), INA3221_REG_CONFIG, buffer, 2));
    return ((buffer[0] << 8) | buffer[1]);
}

float Ina3221::readBusVoltage(ina3221_channel_t channel) {
    uint8_t buffer[2] = {0,};
    ESP_ERROR_CHECK(this->i2c->read(static_cast<uint8_t>(this->address), INA3221_REG_BUS_VOLTAGE + channel * 2, buffer, 2));
    return (((buffer[0] << 8) | buffer[1]) * 0.001);
}

float Ina3221::readCurrent(ina3221_channel_t channel, float shunt) {
    uint8_t buffer[2] = {0,};
    ESP_ERROR_CHECK(this->i2c->read(static_cast<uint8_t>(this->address), INA3221_REG_SHUNT_VOLTAGE + channel * 2, buffer, 2));
    int16_t tmp = (buffer[0] << 8) | buffer[1];
    return (tmp * 5 / shunt);
}
