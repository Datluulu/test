#include "vl53l0x_sensor.h"

#include "esp_log.h"

#include "vl53l0x.h"


static const char *TAG =
    "VL53L0X";


esp_err_t vl53l0x_sensor_init(
    vl53l0x_sensor_handle_t *handle,

    i2c_master_bus_handle_t bus)
{
    if (
        handle == NULL ||
        bus == NULL
    )
    {
        return ESP_ERR_INVALID_ARG;
    }


    /*
     * Create sensor
     */

    vl53l0x_handle_t sensor =
        NULL;


    esp_err_t err =
        vl53l0x_create(
            &sensor,

            bus
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "vl53l0x_create failed: %s",

            esp_err_to_name(err)
        );


        return err;
    }


    /*
     * Init
     */

    err =
        vl53l0x_init(
            sensor
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "vl53l0x_init failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Reference SPAD calibration
     *
     * Đây là calibration cơ bản.
     */

    vl53l0x_ref_spad_calibration_t
        spad_cal = {0};


    err =
        vl53l0x_perform_ref_spad_management(
            sensor,

            &spad_cal
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "SPAD calibration failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Apply SPAD calibration
     */

    err =
        vl53l0x_set_reference_spads(
            sensor,

            &spad_cal
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "Set SPAD calibration failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Reference calibration
     */

    vl53l0x_ref_calibration_t
        ref_cal = {0};


    err =
        vl53l0x_perform_ref_calibration(
            sensor,

            &ref_cal
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "Reference calibration failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Apply calibration
     */

    err =
        vl53l0x_set_ref_calibration(
            sensor,

            &ref_cal
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "Set reference calibration failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Default profile
     */

    err =
        vl53l0x_set_profile(
            sensor,

            VL53L0X_PROFILE_DEFAULT
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "Set profile failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Single measurement
     */

    err =
        vl53l0x_set_mode(
            sensor,

            VL53L0X_MODE_SINGLE
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,

            "Set mode failed: %s",

            esp_err_to_name(err)
        );


        vl53l0x_destroy(
            sensor
        );


        return err;
    }


    /*
     * Save handle
     */

    handle->sensor =
        (void *)sensor;


    ESP_LOGI(
        TAG,

        "VL53L0X initialized"
    );


    return ESP_OK;
}


/*
 * ============================================================
 * READ
 * ============================================================
 */

esp_err_t vl53l0x_sensor_read(
    vl53l0x_sensor_handle_t *handle,

    uint16_t *distance_mm,

    bool *valid,

    uint8_t *range_status)
{
    if (
        handle == NULL ||
        handle->sensor == NULL ||
        distance_mm == NULL ||
        valid == NULL ||
        range_status == NULL
    )
    {
        return ESP_ERR_INVALID_ARG;
    }


    vl53l0x_data_t data = {0};


    esp_err_t err =
        vl53l0x_single_measure(
            (vl53l0x_handle_t)
                handle->sensor,

            &data
        );


    if (err != ESP_OK)
    {
        return err;
    }


    /*
     * Copy result
     */

    *distance_mm =
        data.distance_mm;


    *valid =
        data.valid;


    *range_status =
        data.range_status;


    return ESP_OK;
}