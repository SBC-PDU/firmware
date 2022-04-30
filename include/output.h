#pragma once

#include <driver/gpio.h>

#include "ina3221.h"

class Output {
    public:
        Output(Ina3221 *ina3221, ina3221_channel_t channel, gpio_num_t enable, gpio_num_t alert);
        bool hasAlert();
        void enable(bool enabled);
        float readCurrent();
        float readVoltage();
    private:
        Ina3221 *ina3221;
        ina3221_channel_t channel;
        gpio_num_t alertPin;
        gpio_num_t enablePin;
};