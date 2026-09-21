#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


void control_task_start(
    QueueHandle_t command_queue
);