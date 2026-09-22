#include "app_manager.h"

#include "app_state.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"

#include "sensor_manager.h"
#include "control_task.h"

#include "config.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"


static const char *TAG = "APP";

static i2c_master_bus_handle_t s_i2c_bus = NULL;


/* =========================
 * NVS
 * ========================= */

static esp_err_t init_nvs(void)
{
    esp_err_t err =
        nvs_flash_init();


    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        err = nvs_flash_init();
    }


    return err;
}


/* =========================
 * I2C
 * ========================= */

static esp_err_t init_i2c(void)
{
    i2c_master_bus_config_t config = {
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
        &config,
        &s_i2c_bus
    );
}


/* =========================
 * I2C scan
 * ========================= */

static void i2c_scan(void)
{
    ESP_LOGI(
        TAG,
        "I2C scan..."
    );


    for (uint8_t addr = 1;
         addr < 0x7F;
         addr++)
    {
        esp_err_t err =
            i2c_master_probe(
                s_i2c_bus,
                addr,
                100
            );


        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "Found I2C device: 0x%02X",
                addr
            );
        }
    }


    ESP_LOGI(
        TAG,
        "I2C scan done"
    );
}


/* =========================
 * Wait WiFi
 * ========================= */

static void wait_wifi(void)
{
    while (!wifi_manager_is_connected())
    {
        ESP_LOGI(
            TAG,
            "Waiting WiFi..."
        );

        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}


/* =========================
 * START
 * ========================= */

esp_err_t app_manager_start(void)
{
    ESP_LOGI(
        TAG,
        "ESP32 SENSOR MQTT"
    );


    /* NVS */

    ESP_ERROR_CHECK(
        init_nvs()
    );


    /* Application state */

    ESP_ERROR_CHECK(
        app_state_init()
    );


    /* I2C */

    ESP_ERROR_CHECK(
        init_i2c()
    );


    i2c_scan();


    /* WiFi */

    ESP_ERROR_CHECK(
        wifi_manager_init()
    );


    wait_wifi();


    ESP_LOGI(
        TAG,
        "WiFi connected"
    );


    /* MQTT */

    ESP_ERROR_CHECK(
        mqtt_manager_init()
    );


    /*
     * Control phải init trước
     * để MQTT callback có nơi nhận command.
     */

    ESP_ERROR_CHECK(
        control_task_init()
    );


    /* Sensors */

    ESP_ERROR_CHECK(
        sensor_manager_init(
            s_i2c_bus
        )
    );


    ESP_LOGI(
        TAG,
        "Application ready"
    );


    ESP_LOGI(
        TAG,
        "START: %s",
        MQTT_TOPIC_START
    );


    ESP_LOGI(
        TAG,
        "STOP: %s",
        MQTT_TOPIC_STOP
    );


    /*
     * app_main task không cần làm việc.
     */

    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(10000)
        );
    }


    return ESP_OK;
}