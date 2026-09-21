#include "config.h"

#include "app_state.h"

#include "sensor_data.h"

#include "wifi_manager.h"

#include "mqtt_manager.h"

#include "control_task.h"

#include "sensor_tasks.h"


#include "esp_log.h"

#include "esp_err.h"

#include "nvs_flash.h"


#include "freertos/FreeRTOS.h"

#include "freertos/queue.h"

#include "freertos/task.h"


#include "driver/i2c_master.h"


/*
 * ============================================================
 * GLOBAL APP EVENT GROUP
 * ============================================================
 */

EventGroupHandle_t
g_app_event_group = NULL;


/*
 * ============================================================
 * TAG
 * ============================================================
 */

static const char *TAG =
    "APP";


/*
 * ============================================================
 * I2C BUS
 * ============================================================
 */

static i2c_master_bus_handle_t
s_i2c_bus = NULL;


/*
 * ============================================================
 * NVS INIT
 * ============================================================
 */

static esp_err_t nvs_init(void)
{
    esp_err_t ret =
        nvs_flash_init();


    if (
        ret ==
            ESP_ERR_NVS_NO_FREE_PAGES
        ||
        ret ==
            ESP_ERR_NVS_NEW_VERSION_FOUND
    )
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );


        ret =
            nvs_flash_init();
    }


    return ret;
}


/*
 * ============================================================
 * I2C INIT
 * ============================================================
 */

static esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t bus_config = {

        .i2c_port =
            I2C_PORT,

        .sda_io_num =
            I2C_SDA_GPIO,

        .scl_io_num =
            I2C_SCL_GPIO,

        .clk_source =
            I2C_CLK_SRC_DEFAULT,

        .glitch_ignore_cnt =
            7,

        .flags.enable_internal_pullup =
            true
    };


    return i2c_new_master_bus(
        &bus_config,

        &s_i2c_bus
    );
}


/*
 * ============================================================
 * I2C SCAN
 * ============================================================
 */

static void i2c_scan(void)
{
    ESP_LOGI(
        TAG,

        "I2C scan..."
    );


    for (
        uint8_t address = 1;

        address < 0x7F;

        address++
    )
    {
        esp_err_t err =
            i2c_master_probe(
                s_i2c_bus,

                address,

                100
            );


        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG,

                "I2C device found: 0x%02X",

                address
            );
        }
    }


    ESP_LOGI(
        TAG,

        "I2C scan done"
    );
}


/*
 * ============================================================
 * APP MAIN
 * ============================================================
 */

void app_main(void)
{
    ESP_LOGI(
        TAG,

        "================================"
    );


    ESP_LOGI(
        TAG,

        "ESP32 SENSOR MQTT"
    );


    ESP_LOGI(
        TAG,

        "ESP-IDF 6.x"
    );


    ESP_LOGI(
        TAG,

        "================================"
    );


    /*
     * ========================================================
     * NVS
     * ========================================================
     */

    ESP_ERROR_CHECK(
        nvs_init()
    );


    /*
     * ========================================================
     * APP EVENT GROUP
     * ========================================================
     */

    g_app_event_group =
        xEventGroupCreate();


    if (
        g_app_event_group == NULL
    )
    {
        ESP_LOGE(
            TAG,

            "Cannot create event group"
        );


        abort();
    }


    /*
     * ========================================================
     * I2C
     * ========================================================
     */

    ESP_ERROR_CHECK(
        i2c_init()
    );


    ESP_LOGI(
        TAG,

        "I2C initialized"
    );


    ESP_LOGI(
        TAG,

        "SDA = GPIO%d",

        I2C_SDA_GPIO
    );


    ESP_LOGI(
        TAG,

        "SCL = GPIO%d",

        I2C_SCL_GPIO
    );


    /*
     * Scan I2C
     */

    i2c_scan();


    /*
     * ========================================================
     * WIFI
     * ========================================================
     */

    ESP_ERROR_CHECK(
        wifi_manager_init()
    );


    /*
     * Wait WiFi
     */

    while (
        !wifi_manager_is_connected()
    )
    {
        ESP_LOGI(
            TAG,

            "Waiting for WiFi..."
        );


        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }


    ESP_LOGI(
        TAG,

        "WiFi connected"
    );


    /*
     * ========================================================
     * COMMAND QUEUE
     * ========================================================
     */

    QueueHandle_t
        command_queue =
        xQueueCreate(
            COMMAND_QUEUE_LENGTH,

            sizeof(app_command_t)
        );


    if (
        command_queue == NULL
    )
    {
        ESP_LOGE(
            TAG,

            "Cannot create command queue"
        );


        abort();
    }


    /*
     * ========================================================
     * SENSOR QUEUE
     * ========================================================
     */

    QueueHandle_t
        sensor_queue =
        xQueueCreate(
            SENSOR_QUEUE_LENGTH,

            sizeof(sensor_data_t)
        );


    if (
        sensor_queue == NULL
    )
    {
        ESP_LOGE(
            TAG,

            "Cannot create sensor queue"
        );


        abort();
    }


    /*
     * ========================================================
     * MQTT
     * ========================================================
     *
     * MQTT callback cần command_queue
     * để gửi /start /stop.
     */

    mqtt_manager_set_command_queue(
        command_queue
    );


    ESP_ERROR_CHECK(
        mqtt_manager_init()
    );


    /*
     * ========================================================
     * CONTROL TASK
     * ========================================================
     */

    control_task_start(
        command_queue
    );


    /*
     * ========================================================
     * SENSOR TASKS
     * ========================================================
     */

    sensor_tasks_start(
        sensor_queue,

        s_i2c_bus
    );


    /*
     * ========================================================
     * APPLICATION READY
     * ========================================================
     */

    ESP_LOGI(
        TAG,

        "Application ready"
    );


    ESP_LOGI(
        TAG,

        "Send MQTT %s to start",

        MQTT_TOPIC_START
    );


    ESP_LOGI(
        TAG,

        "Send MQTT %s to stop",

        MQTT_TOPIC_STOP
    );


    /*
     * ========================================================
     * MAIN LOOP
     * ========================================================
     */

    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(10000)
        );
    }
}