#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

#include "driver/i2c_master.h"


typedef struct
{
    void *sensor;

} vl53l0x_sensor_handle_t;


esp_err_t vl53l0x_sensor_init(
    vl53l0x_sensor_handle_t *handle,
    i2c_master_bus_handle_t bus
);


esp_err_t vl53l0x_sensor_read(
    vl53l0x_sensor_handle_t *handle,
    uint16_t *distance_mm,
    bool *valid,
    uint8_t *range_status
);