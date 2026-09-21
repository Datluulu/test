#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/i2c_master.h"


void sensor_tasks_start(
    QueueHandle_t sensor_queue,

    i2c_master_bus_handle_t i2c_bus
);