#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    SENSOR_TYPE_MMA845X = 0,

    SENSOR_TYPE_VL53L0X = 1

} sensor_type_t;


typedef struct
{
    sensor_type_t type;

    int64_t timestamp_us;


    union
    {
        /*
         * MMA845x
         */

        struct
        {
            float x_g;

            float y_g;

            float z_g;

        } mma845x;


        /*
         * VL53L0X
         */

        struct
        {
            uint16_t distance_mm;

            bool valid;

            uint8_t range_status;

        } vl53l0x;

    };

} sensor_data_t;