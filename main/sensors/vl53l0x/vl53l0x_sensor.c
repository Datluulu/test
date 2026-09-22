#include "vl53l0x_sensor.h"

#include "esp_log.h"

#include "vl53l0x.h"


static const char *TAG = "VL53L0X";


esp_err_t vl53l0x_sensor_init(
    vl53l0x_sensor_handle_t *handle,
    i2c_master_bus_handle_t bus)
{
    if (!handle || !bus)
        return ESP_ERR_INVALID_ARG;


    vl53l0x_handle_t sensor = NULL;


    esp_err_t err =
        vl53l0x_create(
            &sensor,
            bus
        );


    if (err != ESP_OK)
        return err;


    err = vl53l0x_init(sensor);

    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    vl53l0x_ref_spad_calibration_t spad = {0};


    err =
        vl53l0x_perform_ref_spad_management(
            sensor,
            &spad
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    err =
        vl53l0x_set_reference_spads(
            sensor,
            &spad
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    vl53l0x_ref_calibration_t ref = {0};


    err =
        vl53l0x_perform_ref_calibration(
            sensor,
            &ref
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    err =
        vl53l0x_set_ref_calibration(
            sensor,
            &ref
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    err =
        vl53l0x_set_profile(
            sensor,
            VL53L0X_PROFILE_DEFAULT
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    err =
        vl53l0x_set_mode(
            sensor,
            VL53L0X_MODE_SINGLE
        );


    if (err != ESP_OK)
    {
        vl53l0x_destroy(sensor);
        return err;
    }


    handle->sensor = sensor;


    ESP_LOGI(
        TAG,
        "VL53L0X initialized"
    );


    return ESP_OK;
}


esp_err_t vl53l0x_sensor_read(
    vl53l0x_sensor_handle_t *handle,
    uint16_t *distance_mm,
    bool *valid,
    uint8_t *range_status)
{
    if (!handle ||
        !handle->sensor ||
        !distance_mm ||
        !valid ||
        !range_status)
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
        return err;


    *distance_mm = data.distance_mm;
    *valid = data.valid;
    *range_status = data.range_status;


    return ESP_OK;
}