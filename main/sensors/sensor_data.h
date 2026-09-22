#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SENSOR_TYPE_MMA845X = 0,
    SENSOR_TYPE_VL53L0X
} sensor_type_t;


typedef struct
{
    float x_g;
    float y_g;
    float z_g;

} mma845x_data_t;


typedef struct
{
    uint16_t distance_mm;
    bool valid;
    uint8_t range_status;

} vl53l0x_data_t;


typedef struct
{
    sensor_type_t type;

    int64_t timestamp_us;

    union
    {
        mma845x_data_t mma845x;
        vl53l0x_data_t vl53l0x;

    };

} sensor_data_t;