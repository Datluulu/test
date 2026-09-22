#include "sensor_manager.h"

#include "sensor_data.h"

#include "mma845x.h"
#include "vl53l0x_sensor.h"

#include "app_state.h"

#include "mqtt_manager.h"

#include "config.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


static const char *TAG = "SENSOR";


static QueueHandle_t s_queue = NULL;

static i2c_master_bus_handle_t s_bus = NULL;


static void send_data(
    const sensor_data_t *data)
{
    if (xQueueSend(
            s_queue,
            data,
            pdMS_TO_TICKS(5)
        ) != pdTRUE)
    {
        ESP_LOGW(
            TAG,
            "Sensor queue full"
        );
    }
}


/* =========================
 * MMA845x
 * ========================= */

static void mma845x_task(void *arg)
{
    mma845x_handle_t sensor = {0};


    esp_err_t err =
        mma845x_init(
            &sensor,
            s_bus
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "MMA845x init failed: %s",
            esp_err_to_name(err)
        );

        vTaskDelete(NULL);
        return;
    }


    ESP_LOGI(
        TAG,
        "MMA845x task ready"
    );


    while (true)
    {
        if (!app_state_is_running())
        {
            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            continue;
        }


        float x;
        float y;
        float z;


        err =
            mma845x_read_accel(
                &sensor,
                &x,
                &y,
                &z
            );


        if (err == ESP_OK)
        {
            sensor_data_t data = {
                .type =
                    SENSOR_TYPE_MMA845X,

                .timestamp_us =
                    esp_timer_get_time(),

                .mma845x = {
                    .x_g = x,
                    .y_g = y,
                    .z_g = z
                }
            };


            send_data(&data);
        }


        vTaskDelay(
            pdMS_TO_TICKS(
                MMA845X_PERIOD_MS
            )
        );
    }
}


/* =========================
 * VL53L0X
 * ========================= */

static void vl53l0x_task(void *arg)
{
    vl53l0x_sensor_handle_t sensor = {
        0
    };


    esp_err_t err =
        vl53l0x_sensor_init(
            &sensor,
            s_bus
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "VL53L0X init failed: %s",
            esp_err_to_name(err)
        );

        vTaskDelete(NULL);
        return;
    }


    ESP_LOGI(
        TAG,
        "VL53L0X task ready"
    );


    while (true)
    {
        if (!app_state_is_running())
        {
            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            continue;
        }


        uint16_t distance = 0;
        bool valid = false;
        uint8_t status = 0;


        err =
            vl53l0x_sensor_read(
                &sensor,
                &distance,
                &valid,
                &status
            );


        if (err == ESP_OK)
        {
            sensor_data_t data = {
                .type =
                    SENSOR_TYPE_VL53L0X,

                .timestamp_us =
                    esp_timer_get_time(),

                .vl53l0x = {
                    .distance_mm =
                        distance,

                    .valid =
                        valid,

                    .range_status =
                        status
                }
            };


            send_data(&data);
        }


        vTaskDelay(
            pdMS_TO_TICKS(
                VL53L0X_PERIOD_MS
            )
        );
    }
}


/* =========================
 * MQTT TX
 * ========================= */

static void mqtt_tx_task(void *arg)
{
    sensor_data_t data;


    ESP_LOGI(
        TAG,
        "MQTT TX ready"
    );


    while (true)
    {
        if (xQueueReceive(
                s_queue,
                &data,
                portMAX_DELAY
            ) == pdTRUE)
        {
            if (!app_state_is_running())
                continue;


            mqtt_publish_sensor_data(
                &data
            );
        }
    }
}


/* =========================
 * INIT
 * ========================= */

esp_err_t sensor_manager_init(
    i2c_master_bus_handle_t bus)
{
    if (!bus)
        return ESP_ERR_INVALID_ARG;


    s_bus = bus;


    s_queue = xQueueCreate(
        SENSOR_QUEUE_LENGTH,
        sizeof(sensor_data_t)
    );


    if (!s_queue)
        return ESP_ERR_NO_MEM;


    if (xTaskCreate(
            mma845x_task,
            "mma845x",
            SENSOR_TASK_STACK,
            NULL,
            SENSOR_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        return ESP_FAIL;
    }


    if (xTaskCreate(
            vl53l0x_task,
            "vl53l0x",
            SENSOR_TASK_STACK,
            NULL,
            SENSOR_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        return ESP_FAIL;
    }


    if (xTaskCreate(
            mqtt_tx_task,
            "sensor_tx",
            MQTT_TASK_STACK,
            NULL,
            MQTT_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        return ESP_FAIL;
    }


    return ESP_OK;
}