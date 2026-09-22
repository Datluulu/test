#pragma once

#include <stdint.h>

/* =========================
 * WiFi
 * ========================= */

#define WIFI_SSID       "PhuongNhi"
#define WIFI_PASSWORD   "datnhikhang"


/* =========================
 * MQTT
 * ========================= */

#define MQTT_BROKER_URI     "mqtt://103.20.102.171:1883"

#define MQTT_USERNAME       ""
#define MQTT_PASSWORD       ""

#define MQTT_CLIENT_ID      "esp32_sensor_01"

#define MQTT_TOPIC_START    "/start"
#define MQTT_TOPIC_STOP     "/stop"
#define MQTT_TOPIC_STATUS   "/status"
#define MQTT_TOPIC_DATA     "/sensor/data"


/* =========================
 * I2C
 * ========================= */

#define I2C_PORT            I2C_NUM_0
#define I2C_SDA_GPIO       21
#define I2C_SCL_GPIO       22
#define I2C_FREQUENCY_HZ   400000


/* =========================
 * MMA845x
 * ========================= */

#define MMA845X_I2C_ADDR    0x1C

#define MMA845X_PERIOD_MS  100


/* =========================
 * VL53L0X
 * ========================= */

#define VL53L0X_PERIOD_MS  100


/* =========================
 * Queue
 * ========================= */

#define COMMAND_QUEUE_LENGTH   8
#define SENSOR_QUEUE_LENGTH    32


/* =========================
 * Task
 * ========================= */

#define CONTROL_TASK_STACK     4096
#define CONTROL_TASK_PRIORITY  5

#define SENSOR_TASK_STACK      4096
#define SENSOR_TASK_PRIORITY   5

#define MQTT_TASK_STACK        4096
#define MQTT_TASK_PRIORITY     5