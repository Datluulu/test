#include "mma845x.h"

#include "config.h"

#include "esp_log.h"

#include <stdint.h>


static const char *TAG = "MMA845X";


#define REG_OUT_X_MSB      0x01
#define REG_WHO_AM_I       0x0D
#define REG_XYZ_DATA_CFG   0x0E
#define REG_CTRL_REG1      0x2A


#define WHO_MMA8451        0x1A
#define WHO_MMA8452        0x2A
#define WHO_MMA8453        0x3A


static esp_err_t write_reg(
    mma845x_handle_t *sensor,
    uint8_t reg,
    uint8_t value)
{
    uint8_t data[2] = {
        reg,
        value
    };


    return i2c_master_transmit(
        sensor->dev,
        data,
        sizeof(data),
        100
    );
}


static esp_err_t read_reg(
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


esp_err_t mma845x_init(
    mma845x_handle_t *sensor,
    i2c_master_bus_handle_t bus)
{
    if (!sensor || !bus)
        return ESP_ERR_INVALID_ARG;


    i2c_device_config_t config = {
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
            &config,
            &sensor->dev
        );


    if (err != ESP_OK)
        return err;


    uint8_t who = 0;


    err = read_reg(
        sensor,
        REG_WHO_AM_I,
        &who
    );


    if (err != ESP_OK)
        return err;


    ESP_LOGI(
        TAG,
        "WHO_AM_I = 0x%02X",
        who
    );


    if (who != WHO_MMA8451 &&
        who != WHO_MMA8452 &&
        who != WHO_MMA8453)
    {
        ESP_LOGE(
            TAG,
            "Unknown device: 0x%02X",
            who
        );

        return ESP_ERR_NOT_FOUND;
    }


    err = write_reg(
        sensor,
        REG_CTRL_REG1,
        0x00
    );

    if (err != ESP_OK)
        return err;


    err = write_reg(
        sensor,
        REG_XYZ_DATA_CFG,
        0x00
    );

    if (err != ESP_OK)
        return err;


    err = write_reg(
        sensor,
        REG_CTRL_REG1,
        0x01
    );

    if (err != ESP_OK)
        return err;


    ESP_LOGI(
        TAG,
        "MMA845x initialized"
    );


    return ESP_OK;
}


esp_err_t mma845x_read_accel(
    mma845x_handle_t *sensor,
    float *x_g,
    float *y_g,
    float *z_g)
{
    if (!sensor ||
        !x_g ||
        !y_g ||
        !z_g)
    {
        return ESP_ERR_INVALID_ARG;
    }


    uint8_t reg = REG_OUT_X_MSB;

    uint8_t data[6];


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
        return err;


    int16_t raw_x =
        (int16_t)(
            ((uint16_t)data[0] << 8)
            | data[1]
        );


    int16_t raw_y =
        (int16_t)(
            ((uint16_t)data[2] << 8)
            | data[3]
        );


    int16_t raw_z =
        (int16_t)(
            ((uint16_t)data[4] << 8)
            | data[5]
        );


    raw_x >>= 2;
    raw_y >>= 2;
    raw_z >>= 2;


    *x_g = (float)raw_x / 1024.0f;
    *y_g = (float)raw_y / 1024.0f;
    *z_g = (float)raw_z / 1024.0f;


    return ESP_OK;
}