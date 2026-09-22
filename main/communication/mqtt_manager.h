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


/* =========================
 * INIT
 * ========================= */

esp_err_t mqtt_manager_init(void);


/* =========================
 * CONNECTION
 * ========================= */

bool mqtt_manager_is_connected(void);


/* =========================
 * COMMAND
 * ========================= */

void mqtt_manager_set_command_callback(
    mqtt_command_callback_t callback
);


/* =========================
 * STATUS
 * ========================= */

void mqtt_publish_status(
    const char *status
);


/* =========================
 * SENSOR DATA
 *
 * Gửi MMA845x + VL53L0X
 * trong MỘT MQTT message.
 * ========================= */

void mqtt_publish_sensor_data_combined(
    const sensor_data_t *mma845x,
    const sensor_data_t *vl53l0x
);