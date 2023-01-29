#pragma once

#include <esp_log.h>
#include <math.h>

#include "i2c_master.h"

typedef enum {
    INA3221_ADDRESS_GND = 0x40,
    INA3221_ADDRESS_VCC,
    INA3221_ADDRESS_SDA,
    INA3221_ADDRESS_SCL,
} ina3221_address_t;

typedef enum {
    INA3221_CHANNEL_1 = 0,
    INA3221_CHANNEL_2,
    INA3221_CHANNEL_3,
} ina3221_channel_t;

enum INA3221_REG {
    /// Configuration register
    INA3221_REG_CONFIG = 0x00,
    /// Shunt voltage register
    INA3221_REG_SHUNT_VOLTAGE = 0x01,
    /// Bus voltage register
    INA3221_REG_BUS_VOLTAGE = 0x02,
};

typedef enum {
    /// Power-down
    INA3221_MODE_POWER_DOWN = 0b000,
    /// Shunt voltage, triggered
    INA3221_MODE_SHUNT_TRIGGERED = 0b001,
    /// Bus voltage, triggered
    INA3221_MODE_BUS_TRIGGERED = 0b010,
    /// Shunt and bus voltage, triggered
    INA3221_MODE_SHUNT_AND_BUS_TRIGGERED = 0b011,
    /// Shunt voltage, continuous
    INA3221_MODE_SHUNT_CONTINUOUS = 0b101,
    /// Bus voltage, continuous
    INA3221_MODE_BUS_CONTINUOUS = 0b110,
    /// Shunt and bus voltage, continuous
    INA3221_MODE_SHUNT_AND_BUS_CONTINUOUS = 0b111,
} ina3221_mode_t;

typedef enum {
    INA3221_SHUNT_CT_140 = 0,
    INA3221_SHUNT_CT_204 = 1 << 3,
    INA3221_SHUNT_CT_332,
    INA3221_SHUNT_CT_588,
    INA3221_SHUNT_CT_1100,  ///< Default
    INA3221_SHUNT_CT_2116,
    INA3221_SHUNT_CT_4156,
    INA3221_SHUNT_CT_8244,
} ina3221_shunt_ct_t;


typedef enum {
    INA3221_BUS_CT_140 = 0,
    INA3221_BUS_CT_204 = 1 << 6,
    INA3221_BUS_CT_332,
    INA3221_BUS_CT_588,
    INA3221_BUS_CT_1100,  ///< Default
    INA3221_BUS_CT_2116,
    INA3221_BUS_CT_4156,
    INA3221_BUS_CT_8244,
} ina3221_bus_ct_t;

typedef enum {
    INA3221_AVG_1 = 0, ///< Default
    INA3221_AVG_4 = 1 << 9,
    INA3221_AVG_16,
    INA3221_AVG_64,
    INA3221_AVG_128,
    INA3221_AVG_256,
    INA3221_AVG_512,
    INA3221_AVG_1024,
} ina3221_avg_t;

typedef enum {
    INA3221_CHANNEL_1_ENABLE = 1 << 14,
    INA3221_CHANNEL_1_DISABLE = 0,
    INA3221_CHANNEL_2_ENABLE = 1 << 13,
    INA3221_CHANNEL_2_DISABLE = 0,
    INA3221_CHANNEL_3_ENABLE = 1 << 12,
    INA3221_CHANNEL_3_DISABLE = 0,
} ina3221_channel_enable_t;

/**
 * @brief TI INA3221 driver
 */
class Ina3221 {
    public:
        /**
         * @brief Construct a new instance of INA3221 driver
         * 
         * @param i2c I2C master driver
         * @param address I2C address
         */
        Ina3221(I2C *i2c, ina3221_address_t address);

        /**
         * @brief Writes a configuration into INA3221
         * 
         * @param config Configuration to write
         */
        void writeConfiguration(uint16_t config);

        /**
         * @brief Reads a configuration from INA3221
         * 
         * @return uint16_t Configuration
         */
        uint16_t readConfiguration();

        /**
         * @brief Reads the bus voltage at specified channel
         * 
         * @param channel Channel
         * @return float Bus voltage
         */
        float readBusVoltage(ina3221_channel_t channel);

        /**
         * @brief Reads the current at specified channel
         * 
         * @param channel Channel
         * @param shunt Shunt resistor value
         * @return float Current
         */
        float readCurrent(ina3221_channel_t channel, float shunt);
    private:
        /// @brief Pointer to I2C master instance
        I2C *i2c;
        /// @brief I2C address
        ina3221_address_t address;
};