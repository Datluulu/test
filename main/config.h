#pragma once

/*
 * ============================================================
 * WIFI
 * ============================================================
 */

#define WIFI_SSID       "PhuongNhi"
#define WIFI_PASSWORD   "datnhikhang"


/*
 * ============================================================
 * MQTT
 * ============================================================
 *
 * Ví dụ Mosquitto:
 *
 * mqtt://192.168.1.100:1883
 *
 */
//http://103.20.102.171/
#define MQTT_BROKER_URI     "mqtt://103.20.102.171:1883"

#define MQTT_CLIENT_ID      "esp32_sensor_01"

#define MQTT_USERNAME       ""
#define MQTT_PASSWORD       ""


/*
 * ============================================================
 * MQTT TOPICS
 * ============================================================
 */

#define MQTT_TOPIC_START    "/start"
#define MQTT_TOPIC_STOP     "/stop"

#define MQTT_TOPIC_DATA     "esp32/sensor/data"
#define MQTT_TOPIC_STATUS   "esp32/status"


/*
 * ============================================================
 * I2C
 * ============================================================
 */

#define I2C_PORT            I2C_NUM_0

#define I2C_SDA_GPIO        21
#define I2C_SCL_GPIO        22

#define I2C_FREQUENCY_HZ    400000


/*
 * ============================================================
 * MMA845x
 * ============================================================
 *
 * MMA845x có thể là 0x1C hoặc 0x1D.
 *
 * Kiểm tra SA0.
 *
 */

#define MMA845X_I2C_ADDR    0x1C


/*
 * ============================================================
 * VL53L0X
 * ============================================================
 */

#define VL53L0X_I2C_ADDR    0x29


/*
 * ============================================================
 * QUEUE
 * ============================================================
 */

#define SENSOR_QUEUE_LENGTH     32

#define COMMAND_QUEUE_LENGTH    8


/*
 * ============================================================
 * SENSOR SAMPLING
 * ============================================================
 *
 * MMA845x:
 * 20 ms = 50 Hz
 *
 * VL53L0X:
 * 100 ms = 10 Hz
 *
 */

#define MMA845X_PERIOD_MS       2000

#define VL53L0X_PERIOD_MS       2000


/*
 * ============================================================
 * TASK STACK
 * ============================================================
 */

#define SENSOR_TASK_STACK       4096

#define MQTT_TASK_STACK         4096

#define CONTROL_TASK_STACK      3072


/*
 * ============================================================
 * TASK PRIORITY
 * ============================================================
 */

#define SENSOR_TASK_PRIORITY    5

#define MQTT_TASK_PRIORITY      6

#define CONTROL_TASK_PRIORITY   7