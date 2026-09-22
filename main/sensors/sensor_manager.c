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


/* =========================
 * QUEUE
 * ========================= */

static QueueHandle_t s_queue = NULL;


/* =========================
 * I2C BUS
 * ========================= */

static i2c_master_bus_handle_t s_bus = NULL;


/* =========================
 * SEND SENSOR DATA
 * ========================= */

static void send_data(
    const sensor_data_t *data)
{
    if (data == NULL)
    {
        return;
    }


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
 * MMA845x TASK
 * ========================= */

static void mma845x_task(
    void *arg)
{
    mma845x_handle_t sensor = {
        0
    };


    /*
     * Init sensor
     */

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
        /*
         * Nếu STOP
         * thì không đọc sensor.
         */

        if (!app_state_is_running())
        {
            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            continue;
        }


        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;


        /*
         * Read MMA845x
         */

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


            /*
             * Đẩy vào queue.
             *
             * MQTT task sẽ gom
             * với VL53L0X.
             */

            send_data(&data);
        }
        else
        {
            ESP_LOGW(
                TAG,
                "MMA845x read failed: %s",
                esp_err_to_name(err)
            );
        }


        /*
         * Sampling period
         */

        vTaskDelay(
            pdMS_TO_TICKS(
                MMA845X_PERIOD_MS
            )
        );
    }
}


/* =========================
 * VL53L0X TASK
 * ========================= */

static void vl53l0x_task(
    void *arg)
{
    vl53l0x_sensor_handle_t sensor = {
        0
    };


    /*
     * Init sensor
     */

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
        /*
         * Nếu STOP
         * thì không đọc sensor.
         */

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


        /*
         * Read VL53L0X
         */

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


            /*
             * Đẩy vào queue.
             */

            send_data(&data);
        }
        else
        {
            ESP_LOGW(
                TAG,
                "VL53L0X read failed: %s",
                esp_err_to_name(err)
            );
        }


        /*
         * Sampling period
         */

        vTaskDelay(
            pdMS_TO_TICKS(
                VL53L0X_PERIOD_MS
            )
        );
    }
}


/* =========================
 * MQTT TX TASK
 *
 * GOM 2 SENSOR
 * ========================= */

static void mqtt_tx_task(
    void *arg)
{
    sensor_data_t data;


    /*
     * Dữ liệu mới nhất
     * của MMA845x.
     */

    sensor_data_t mma845x_data = {
        0
    };


    /*
     * Dữ liệu mới nhất
     * của VL53L0X.
     */

    sensor_data_t vl53l0x_data = {
        0
    };


    /*
     * Flag kiểm tra
     * đã nhận sensor chưa.
     */

    bool have_mma845x = false;

    bool have_vl53l0x = false;


    ESP_LOGI(
        TAG,
        "MQTT TX ready"
    );


    while (true)
    {
        /*
         * Chờ dữ liệu sensor.
         */

        if (xQueueReceive(
                s_queue,
                &data,
                portMAX_DELAY
            ) != pdTRUE)
        {
            continue;
        }


        /*
         * Nếu STOP:
         *
         * - Không publish.
         * - Xóa dữ liệu cũ.
         *
         * Khi START lại,
         * phải lấy lại cả 2 sensor.
         */

        if (!app_state_is_running())
        {
            have_mma845x = false;

            have_vl53l0x = false;

            continue;
        }


        /* =====================
         * MMA845x
         * ===================== */

        if (data.type ==
            SENSOR_TYPE_MMA845X)
        {
            /*
             * Lưu giá trị mới nhất.
             */

            mma845x_data = data;

            have_mma845x = true;
        }


        /* =====================
         * VL53L0X
         * ===================== */

        else if (data.type ==
                 SENSOR_TYPE_VL53L0X)
        {
            /*
             * Lưu giá trị mới nhất.
             */

            vl53l0x_data = data;

            have_vl53l0x = true;
        }


        /*
         * ======================
         * ĐỦ 2 SENSOR
         * ======================
         *
         * Chỉ publish 1 MQTT.
         */

        if (have_mma845x &&
            have_vl53l0x)
        {
            mqtt_publish_sensor_data_combined(
                &mma845x_data,
                &vl53l0x_data
            );


            /*
             * Reset.
             *
             * Chờ dữ liệu mới của
             * cả 2 sensor.
             */

            have_mma845x = false;

            have_vl53l0x = false;
        }
    }
}


/* =========================
 * INIT
 * ========================= */

esp_err_t sensor_manager_init(
    i2c_master_bus_handle_t bus)
{
    /*
     * Check I2C bus
     */

    if (bus == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }


    s_bus = bus;


    /*
     * Create sensor queue
     */

    s_queue = xQueueCreate(
        SENSOR_QUEUE_LENGTH,
        sizeof(sensor_data_t)
    );


    if (s_queue == NULL)
    {
        return ESP_ERR_NO_MEM;
    }


    /* =====================
     * MMA845x TASK
     * ===================== */

    if (xTaskCreate(
            mma845x_task,
            "mma845x",
            SENSOR_TASK_STACK,
            NULL,
            SENSOR_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create MMA845x task"
        );

        return ESP_FAIL;
    }


    /* =====================
     * VL53L0X TASK
     * ===================== */

    if (xTaskCreate(
            vl53l0x_task,
            "vl53l0x",
            SENSOR_TASK_STACK,
            NULL,
            SENSOR_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create VL53L0X task"
        );

        return ESP_FAIL;
    }


    /* =====================
     * MQTT TX TASK
     * ===================== */

    if (xTaskCreate(
            mqtt_tx_task,
            "sensor_tx",
            MQTT_TASK_STACK,
            NULL,
            MQTT_TASK_PRIORITY,
            NULL
        ) != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "Failed to create MQTT TX task"
        );

        return ESP_FAIL;
    }


    ESP_LOGI(
        TAG,
        "Sensor manager ready"
    );


    return ESP_OK;
}