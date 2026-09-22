#include "mqtt_manager.h"

#include "config.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "mqtt_client.h"

#include <string.h>
#include <stdio.h>


static const char *TAG = "MQTT";


static esp_mqtt_client_handle_t s_client = NULL;


static volatile bool s_connected = false;


static mqtt_command_callback_t s_command_callback = NULL;


/* =========================
 * COMMAND CALLBACK
 * ========================= */

void mqtt_manager_set_command_callback(
    mqtt_command_callback_t callback)
{
    s_command_callback = callback;
}


/* =========================
 * PROCESS COMMAND
 * ========================= */

static void process_command(
    const char *topic,
    int topic_len)
{
    if (topic == NULL)
    {
        return;
    }


    /*
     * START
     */

    if (topic_len == strlen(MQTT_TOPIC_START) &&
        strncmp(
            topic,
            MQTT_TOPIC_START,
            topic_len
        ) == 0)
    {
        if (s_command_callback)
        {
            s_command_callback(
                MQTT_COMMAND_START
            );
        }


        ESP_LOGI(
            TAG,
            "START received"
        );

        return;
    }


    /*
     * STOP
     */

    if (topic_len == strlen(MQTT_TOPIC_STOP) &&
        strncmp(
            topic,
            MQTT_TOPIC_STOP,
            topic_len
        ) == 0)
    {
        if (s_command_callback)
        {
            s_command_callback(
                MQTT_COMMAND_STOP
            );
        }


        ESP_LOGI(
            TAG,
            "STOP received"
        );

        return;
    }
}


/* =========================
 * MQTT EVENT
 * ========================= */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        event_data;


    switch (
        (esp_mqtt_event_id_t)event_id
    )
    {
        /* =====================
         * CONNECTED
         * ===================== */

        case MQTT_EVENT_CONNECTED:

            s_connected = true;


            ESP_LOGI(
                TAG,
                "MQTT connected"
            );


            /*
             * Subscribe START
             */

            esp_mqtt_client_subscribe(
                s_client,
                MQTT_TOPIC_START,
                1
            );


            /*
             * Subscribe STOP
             */

            esp_mqtt_client_subscribe(
                s_client,
                MQTT_TOPIC_STOP,
                1
            );


            /*
             * Online status
             */

            mqtt_publish_status(
                "online"
            );

            break;


        /* =====================
         * DISCONNECTED
         * ===================== */

        case MQTT_EVENT_DISCONNECTED:

            s_connected = false;


            ESP_LOGW(
                TAG,
                "MQTT disconnected"
            );

            break;


        /* =====================
         * DATA
         * ===================== */

        case MQTT_EVENT_DATA:

            process_command(
                event->topic,
                event->topic_len
            );

            break;


        /* =====================
         * ERROR
         * ===================== */

        case MQTT_EVENT_ERROR:

            ESP_LOGE(
                TAG,
                "MQTT error"
            );

            break;


        default:

            break;
    }
}


/* =========================
 * INIT
 * ========================= */

esp_err_t mqtt_manager_init(void)
{
    esp_mqtt_client_config_t config = {

        .broker.address.uri =
            MQTT_BROKER_URI
    };


    /*
     * Username
     */

    if (MQTT_USERNAME[0] != '\0')
    {
        config.credentials.username =
            MQTT_USERNAME;
    }


    /*
     * Password
     */

    if (MQTT_PASSWORD[0] != '\0')
    {
        config.credentials.authentication.password =
            MQTT_PASSWORD;
    }


    /*
     * Client ID
     */

    if (MQTT_CLIENT_ID[0] != '\0')
    {
        config.credentials.client_id =
            MQTT_CLIENT_ID;
    }


    /*
     * Create MQTT client
     */

    s_client =
        esp_mqtt_client_init(
            &config
        );


    if (s_client == NULL)
    {
        ESP_LOGE(
            TAG,
            "MQTT client init failed"
        );

        return ESP_FAIL;
    }


    /*
     * Register event
     */

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            s_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );


    /*
     * Start
     */

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(
            s_client
        )
    );


    ESP_LOGI(
        TAG,
        "MQTT started: %s",
        MQTT_BROKER_URI
    );


    return ESP_OK;
}


/* =========================
 * CONNECTION STATUS
 * ========================= */

bool mqtt_manager_is_connected(void)
{
    return s_connected;
}


/* =========================
 * PUBLISH STATUS
 * ========================= */

void mqtt_publish_status(
    const char *status)
{
    if (!s_connected)
    {
        return;
    }


    if (status == NULL)
    {
        return;
    }


    char payload[160];


    int len = snprintf(
        payload,
        sizeof(payload),

        "{"
        "\"device\":\"%s\","
        "\"status\":\"%s\""
        "}",

        MQTT_CLIENT_ID,
        status
    );


    if (len <= 0 ||
        len >= sizeof(payload))
    {
        return;
    }


    esp_mqtt_client_publish(
        s_client,
        MQTT_TOPIC_STATUS,
        payload,
        len,
        1,
        0
    );
}


/* =========================
 * PUBLISH BOTH SENSORS
 *
 * ONE MQTT MESSAGE
 * ========================= */

void mqtt_publish_sensor_data_combined(
    const sensor_data_t *mma845x,
    const sensor_data_t *vl53l0x)
{
    /*
     * MQTT chưa connected
     */

    if (!s_connected)
    {
        return;
    }


    /*
     * Check pointer
     */

    if (mma845x == NULL ||
        vl53l0x == NULL)
    {
        return;
    }


    char payload[512];


    /*
     * Timestamp mới cho
     * message MQTT.
     */

    int64_t timestamp =
        esp_timer_get_time();


    /*
     * Tạo JSON
     *
     * Một message chứa
     * cả MMA845x và VL53L0X.
     */

    int len = snprintf(
        payload,
        sizeof(payload),

        "{"

        "\"device\":\"%s\","

        "\"timestamp_us\":%lld,"

        "\"mma845x\":{"

            "\"x_g\":%.5f,"
            "\"y_g\":%.5f,"
            "\"z_g\":%.5f"

        "},"

        "\"vl53l0x\":{"

            "\"distance_mm\":%u,"
            "\"valid\":%s,"
            "\"range_status\":%u"

        "}"

        "}",


        /*
         * Device
         */

        MQTT_CLIENT_ID,


        /*
         * Timestamp
         */

        timestamp,


        /*
         * MMA845x
         */

        mma845x->mma845x.x_g,

        mma845x->mma845x.y_g,

        mma845x->mma845x.z_g,


        /*
         * VL53L0X
         */

        vl53l0x->vl53l0x.distance_mm,

        vl53l0x->vl53l0x.valid
            ? "true"
            : "false",

        vl53l0x->vl53l0x.range_status
    );


    /*
     * Check payload
     */

    if (len <= 0 ||
        len >= sizeof(payload))
    {
        ESP_LOGE(
            TAG,
            "MQTT payload too large"
        );

        return;
    }


    /*
     * Publish ONE message
     */

    int msg_id =
        esp_mqtt_client_publish(
            s_client,
            MQTT_TOPIC_DATA,
            payload,
            len,
            1,
            0
        );


    ESP_LOGI(
        TAG,
        "Published combined data, msg_id=%d",
        msg_id
    );


    ESP_LOGI(
        TAG,
        "Payload: %s",
        payload
    );
}