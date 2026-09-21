#include "mma845x.h"

#include "config.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdint.h>


static const char *TAG =
    "MMA845X";


/*
 * Registers
 */

#define REG_OUT_X_MSB      0x01

#define REG_WHO_AM_I       0x0D

#define REG_XYZ_DATA_CFG   0x0E

#define REG_CTRL_REG1      0x2A


/*
 * WHO_AM_I
 */

#define WHO_MMA8451        0x1A

#define WHO_MMA8452        0x2A

#define WHO_MMA8453        0x3A


/*
 * ============================================================
 * WRITE REGISTER
 * ============================================================
 */

static esp_err_t mma_write_reg(
    mma845x_handle_t *sensor,

    uint8_t reg,

    uint8_t value)
{
    uint8_t tx[2] = {
        reg,
        value
    };


    return i2c_master_transmit(
        sensor->dev,

        tx,

        sizeof(tx),

        100
    );
}


/*
 * ============================================================
 * READ REGISTER
 * ============================================================
 */

static esp_err_t mma_read_reg(
    mma845x_handle_t *sensor,

    uint8_t reg,

    uint8_t *value)
{
    return i2c_master_transmit_receive(
        sensor->dev,

        &reg,

        1,

        value,

        1,

        100
    );
}


/*
 * ============================================================
 * INIT
 * ============================================================
 */

esp_err_t mma845x_init(
    mma845x_handle_t *sensor,

    i2c_master_bus_handle_t bus)
{
    if (
        sensor == NULL ||
        bus == NULL
    )
    {
        return ESP_ERR_INVALID_ARG;
    }


    /*
     * Add I2C device
     */

    i2c_device_config_t dev_cfg = {

        .dev_addr_length =
            I2C_ADDR_BIT_LEN_7,

        .device_address =
            MMA845X_I2C_ADDR,

        .scl_speed_hz =
            I2C_FREQUENCY_HZ
    };


    esp_err_t err =
        i2c_master_bus_add_device(
            bus,

            &dev_cfg,

            &sensor->dev
        );


    if (err != ESP_OK)
    {
        return err;
    }


    /*
     * WHO AM I
     */

    uint8_t who = 0;


    err =
        mma_read_reg(
            sensor,

            REG_WHO_AM_I,

            &who
        );


    if (err != ESP_OK)
    {
        return err;
    }


    ESP_LOGI(
        TAG,

        "WHO_AM_I = 0x%02X",

        who
    );


    /*
     * Check device
     */

    if (
        who != WHO_MMA8451 &&
        who != WHO_MMA8452 &&
        who != WHO_MMA8453
    )
    {
        ESP_LOGE(
            TAG,

            "Unknown MMA845x: 0x%02X",

            who
        );


        return ESP_ERR_NOT_FOUND;
    }


    /*
     * ============================================
     * STANDBY
     * ============================================
     */

    ESP_ERROR_CHECK(
        mma_write_reg(
            sensor,

            REG_CTRL_REG1,

            0x00
        )
    );


    /*
     * ============================================
     * RANGE = +/-2g
     * ============================================
     */

    ESP_ERROR_CHECK(
        mma_write_reg(
            sensor,

            REG_XYZ_DATA_CFG,

            0x00
        )
    );


    /*
     * ============================================
     * ACTIVE
     * ============================================
     */

    ESP_ERROR_CHECK(
        mma_write_reg(
            sensor,

            REG_CTRL_REG1,

            0x01
        )
    );


    vTaskDelay(
        pdMS_TO_TICKS(10)
    );


    ESP_LOGI(
        TAG,

        "MMA845x initialized"
    );


    return ESP_OK;
}


/*
 * ============================================================
 * READ ACCEL
 * ============================================================
 */

esp_err_t mma845x_read_accel(
    mma845x_handle_t *sensor,

    float *x_g,

    float *y_g,

    float *z_g)
{
    if (
        sensor == NULL ||
        x_g == NULL ||
        y_g == NULL ||
        z_g == NULL
    )
    {
        return ESP_ERR_INVALID_ARG;
    }


    uint8_t reg =
        REG_OUT_X_MSB;


    uint8_t data[6];


    /*
     * Read X/Y/Z
     */

    esp_err_t err =
        i2c_master_transmit_receive(
            sensor->dev,

            &reg,

            1,

            data,

            sizeof(data),

            100
        );


    if (err != ESP_OK)
    {
        return err;
    }


    /*
     * Convert raw
     */

    int16_t raw_x =
        (int16_t)(
            ((uint16_t)data[0] << 8)
            |
            data[1]
        );


    int16_t raw_y =
        (int16_t)(
            ((uint16_t)data[2] << 8)
            |
            data[3]
        );


    int16_t raw_z =
        (int16_t)(
            ((uint16_t)data[4] << 8)
            |
            data[5]
        );


    /*
     * 14-bit left aligned
     */

    raw_x >>= 2;

    raw_y >>= 2;

    raw_z >>= 2;


    /*
     * +/-2g
     *
     * approximately
     * 1024 counts / g
     */

    *x_g =
        (float)raw_x /
        1024.0f;


    *y_g =
        (float)raw_y /
        1024.0f;


    *z_g =
        (float)raw_z /
        1024.0f;


    return ESP_OK;
}