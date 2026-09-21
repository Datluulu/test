#include "sensor_tasks.h"

#include "config.h"

#include "app_state.h"

#include "sensor_data.h"

#include "mma845x.h"

#include "vl53l0x_sensor.h"

#include "mqtt_manager.h"

#include "esp_log.h"

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


static const char *TAG =
    "SENSOR_TASK";


/*
 * Queue sensor
 */

static QueueHandle_t
s_sensor_queue = NULL;


/*
 * I2C bus
 */

static i2c_master_bus_handle_t
s_i2c_bus = NULL;


/*
 * ============================================================
 * CHECK RUNNING
 * ============================================================
 */

static bool app_is_running(void)
{
    EventBits_t bits =
        xEventGroupGetBits(
            g_app_event_group
        );


    return (
        bits &
        APP_RUNNING_BIT
    ) != 0;
}


/*
 * ============================================================
 * MMA845x TASK
 * ============================================================
 */

static void mma845x_task(
    void *arg)
{
    mma845x_handle_t sensor = {0};


    /*
     * Init
     */

    esp_err_t err =
        mma845x_init(
            &sensor,

            s_i2c_bus
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

        "MMA845x task started"
    );


    /*
     * Main loop
     */

    while (true)
    {
        /*
         * Chưa START
         */

        if (!app_is_running())
        {
            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            continue;
        }


        /*
         * Read accelerometer
         */

        float x_g = 0;

        float y_g = 0;

        float z_g = 0;


        err =
            mma845x_read_accel(
                &sensor,

                &x_g,

                &y_g,

                &z_g
            );


        if (err == ESP_OK)
        {
            /*
             * Build data packet
             */

            sensor_data_t data = {

                .type =
                    SENSOR_TYPE_MMA845X,

                .timestamp_us =
                    esp_timer_get_time()
            };


            data.mma845x.x_g =
                x_g;


            data.mma845x.y_g =
                y_g;


            data.mma845x.z_g =
                z_g;


            /*
             * Send Queue
             */

            if (
                xQueueSend(
                    s_sensor_queue,

                    &data,

                    pdMS_TO_TICKS(5)
                )
                != pdTRUE
            )
            {
                ESP_LOGW(
                    TAG,

                    "sensor_queue full - drop MMA845x"
                );
            }
        }
        else
        {
            ESP_LOGW(
                TAG,

                "MMA845x read error: %s",

                esp_err_to_name(err)
            );
        }


        /*
         * Sampling rate
         */

        vTaskDelay(
            pdMS_TO_TICKS(
                MMA845X_PERIOD_MS
            )
        );
    }
}


/*
 * ============================================================
 * VL53L0X TASK
 * ============================================================
 */

static void vl53l0x_task(
    void *arg)
{
    vl53l0x_sensor_handle_t sensor =
        {0};


    /*
     * Init
     */

    esp_err_t err =
        vl53l0x_sensor_init(
            &sensor,

            s_i2c_bus
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

        "VL53L0X task started"
    );


    /*
     * Main loop
     */

    while (true)
    {
        /*
         * Chưa START
         */

        if (!app_is_running())
        {
            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            continue;
        }


        /*
         * Read distance
         */

        uint16_t distance_mm = 0;

        bool valid = false;

        uint8_t range_status = 0;


        err =
            vl53l0x_sensor_read(
                &sensor,

                &distance_mm,

                &valid,

                &range_status
            );


        if (err == ESP_OK)
        {
            /*
             * Build packet
             */

            sensor_data_t data = {

                .type =
                    SENSOR_TYPE_VL53L0X,

                .timestamp_us =
                    esp_timer_get_time()
            };


            data.vl53l0x.distance_mm =
                distance_mm;


            data.vl53l0x.valid =
                valid;


            data.vl53l0x.range_status =
                range_status;


            /*
             * Queue
             */

            if (
                xQueueSend(
                    s_sensor_queue,

                    &data,

                    pdMS_TO_TICKS(5)
                )
                != pdTRUE
            )
            {
                ESP_LOGW(
                    TAG,

                    "sensor_queue full - drop VL53L0X"
                );
            }
        }
        else
        {
            ESP_LOGW(
                TAG,

                "VL53L0X read error: %s",

                esp_err_to_name(err)
            );
        }


        /*
         * Sampling rate
         */

        vTaskDelay(
            pdMS_TO_TICKS(
                VL53L0X_PERIOD_MS
            )
        );
    }
}


/*
 * ============================================================
 * MQTT PUBLISH TASK
 * ============================================================
 */

static void mqtt_publish_task(
    void *arg)
{
    sensor_data_t data;


    ESP_LOGI(
        TAG,

        "MQTT publish task started"
    );


    while (true)
    {
        /*
         * Block until data arrives.
         */

        if (
            xQueueReceive(
                s_sensor_queue,

                &data,

                portMAX_DELAY
            )
            == pdTRUE
        )
        {
            /*
             * Nếu STOP xảy ra trong lúc
             * data đang nằm trong queue,
             * bỏ packet đó.
             */

            if (!app_is_running())
            {
                continue;
            }


            /*
             * MQTT publish
             */

            mqtt_publish_sensor_data(
                &data
            );
        }
    }
}


/*
 * ============================================================
 * START ALL SENSOR TASKS
 * ============================================================
 */

void sensor_tasks_start(
    QueueHandle_t sensor_queue,

    i2c_master_bus_handle_t i2c_bus)
{
    s_sensor_queue =
        sensor_queue;


    s_i2c_bus =
        i2c_bus;


    /*
     * MMA845x
     */

    xTaskCreate(
        mma845x_task,

        "mma845x_task",

        SENSOR_TASK_STACK,

        NULL,

        SENSOR_TASK_PRIORITY,

        NULL
    );


    /*
     * VL53L0X
     */

    xTaskCreate(
        vl53l0x_task,

        "vl53l0x_task",

        SENSOR_TASK_STACK,

        NULL,

        SENSOR_TASK_PRIORITY,

        NULL
    );


    /*
     * MQTT TX
     */

    xTaskCreate(
        mqtt_publish_task,

        "mqtt_publish_task",

        MQTT_TASK_STACK,

        NULL,

        MQTT_TASK_PRIORITY,

        NULL
    );
}