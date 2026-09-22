#pragma once

#include <stdint.h>

#include "esp_err.h"

#include "driver/i2c_master.h"


typedef struct
{
    i2c_master_dev_handle_t dev;

} mma845x_handle_t;


esp_err_t mma845x_init(
    mma845x_handle_t *sensor,
    i2c_master_bus_handle_t bus
);


esp_err_t mma845x_read_accel(
    mma845x_handle_t *sensor,
    float *x_g,
    float *y_g,
    float *z_g
);