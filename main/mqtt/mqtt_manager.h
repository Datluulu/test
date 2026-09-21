#pragma once

#include <stdbool.h>

#include "esp_err.h"

#include "sensor_data.h"


typedef enum
{
    APP_CMD_START = 0,

    APP_CMD_STOP

} app_command_type_t;


typedef struct
{
    app_command_type_t type;

} app_command_t;


/*
 * Init MQTT
 */

esp_err_t mqtt_manager_init(void);


/*
 * MQTT connected?
 */

bool mqtt_manager_is_connected(void);


/*
 * Đưa command queue cho MQTT
 */

void mqtt_manager_set_command_queue(
    void *queue
);


/*
 * Publish sensor
 */

void mqtt_publish_sensor_data(
    const sensor_data_t *data
);


/*
 * Publish status
 */

void mqtt_publish_status(
    const char *status
);