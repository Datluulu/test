#pragma once

#include <stdbool.h>

#include "esp_err.h"

#include "sensor_data.h"


typedef enum
{
    MQTT_COMMAND_START = 0,
    MQTT_COMMAND_STOP

} mqtt_command_t;


typedef void (*mqtt_command_callback_t)(
    mqtt_command_t command
);


esp_err_t mqtt_manager_init(void);

void mqtt_manager_set_command_callback(
    mqtt_command_callback_t callback
);

bool mqtt_manager_is_connected(void);

void mqtt_publish_status(
    const char *status
);

void mqtt_publish_sensor_data(
    const sensor_data_t *data
);