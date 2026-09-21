#include "mqtt_manager.h"

#include "config.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <string.h>
#include <stdio.h>


static const char *TAG = "MQTT";


static esp_mqtt_client_handle_t
s_mqtt_client = NULL;


static volatile bool
s_mqtt_connected = false;


static QueueHandle_t
s_command_queue = NULL;


/*
 * ============================================================
 * Set command queue
 * ============================================================
 */

void mqtt_manager_set_command_queue(
    void *queue)
{
    s_command_queue =
        (QueueHandle_t)queue;
}


/*
 * ============================================================
 * Process MQTT topic
 * ============================================================
 */

static void mqtt_process_command(
    const char *topic,
    int topic_len)
{
    /*
     * /start
     */

    if (
        topic_len ==
            (int)strlen(
                MQTT_TOPIC_START
            )
        &&
        strncmp(
            topic,
            MQTT_TOPIC_START,
            topic_len
        ) == 0
    )
    {
        app_command_t command = {
            .type = APP_CMD_START
        };


        if (s_command_queue != NULL)
        {
            xQueueSend(
                s_command_queue,
                &command,
                0
            );
        }


        ESP_LOGI(
            TAG,
            "Received START command"
        );


        return;
    }


    /*
     * /stop
     */

    if (
        topic_len ==
            (int)strlen(
                MQTT_TOPIC_STOP
            )
        &&
        strncmp(
            topic,
            MQTT_TOPIC_STOP,
            topic_len
        ) == 0
    )
    {
        app_command_t command = {
            .type = APP_CMD_STOP
        };


        if (s_command_queue != NULL)
        {
            xQueueSend(
                s_command_queue,
                &command,
                0
            );
        }


        ESP_LOGI(
            TAG,
            "Received STOP command"
        );


        return;
    }
}


/*
 * ============================================================
 * MQTT callback
 * ============================================================
 */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)
        event_data;


    switch (
        (esp_mqtt_event_id_t)event_id
    )
    {
        /*
         * CONNECTED
         */

        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI(
                TAG,
                "MQTT connected"
            );


            s_mqtt_connected = true;


            /*
             * Subscribe /start
             */

            esp_mqtt_client_subscribe(
                s_mqtt_client,
                MQTT_TOPIC_START,
                1
            );


            /*
             * Subscribe /stop
             */

            esp_mqtt_client_subscribe(
                s_mqtt_client,
                MQTT_TOPIC_STOP,
                1
            );


            ESP_LOGI(
                TAG,
                "Subscribed %s",
                MQTT_TOPIC_START
            );


            ESP_LOGI(
                TAG,
                "Subscribed %s",
                MQTT_TOPIC_STOP
            );


            mqtt_publish_status(
                "online"
            );


            break;
        }


        /*
         * DISCONNECTED
         */

        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGW(
                TAG,
                "MQTT disconnected"
            );


            s_mqtt_connected = false;


            break;
        }


        /*
         * MQTT DATA
         */

        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(
                TAG,
                "RX topic=%.*s",
                event->topic_len,
                event->topic
            );


            ESP_LOGI(
                TAG,
                "RX data=%.*s",
                event->data_len,
                event->data
            );


            /*
             * Topic không nhất thiết null-terminated.
             */

            mqtt_process_command(
                event->topic,
                event->topic_len
            );


            break;
        }


        case MQTT_EVENT_ERROR:
        {
            ESP_LOGE(
                TAG,
                "MQTT error"
            );


            break;
        }


        default:
            break;
    }
}


/*
 * ============================================================
 * MQTT INIT
 * ============================================================
 */

esp_err_t mqtt_manager_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri =
            MQTT_BROKER_URI,
    };


    /*
     * Username
     */

    if (MQTT_USERNAME[0] != '\0')
    {
        mqtt_cfg.credentials.username =
            MQTT_USERNAME;
    }


    /*
     * Password
     */

    if (MQTT_PASSWORD[0] != '\0')
    {
        mqtt_cfg.credentials.authentication.password =
            MQTT_PASSWORD;
    }


    /*
     * Client ID
     */

    if (MQTT_CLIENT_ID[0] != '\0')
    {
        mqtt_cfg.credentials.client_id =
            MQTT_CLIENT_ID;
    }


    /*
     * Create client
     */

    s_mqtt_client =
        esp_mqtt_client_init(
            &mqtt_cfg
        );


    if (s_mqtt_client == NULL)
    {
        ESP_LOGE(
            TAG,
            "esp_mqtt_client_init failed"
        );

        return ESP_FAIL;
    }


    /*
     * Register event
     */

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            s_mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );


    /*
     * Start MQTT
     */

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(
            s_mqtt_client
        )
    );


    ESP_LOGI(
        TAG,
        "MQTT started: %s",
        MQTT_BROKER_URI
    );


    return ESP_OK;
}


/*
 * ============================================================
 * Connected?
 * ============================================================
 */

bool mqtt_manager_is_connected(void)
{
    return s_mqtt_connected;
}


/*
 * ============================================================
 * STATUS
 * ============================================================
 */

void mqtt_publish_status(
    const char *status)
{
    if (
        !s_mqtt_connected ||
        status == NULL
    )
    {
        return;
    }


    char payload[160];


    int len =
        snprintf(
            payload,
            sizeof(payload),

            "{"
            "\"device\":\"%s\","
            "\"status\":\"%s\""
            "}",

            MQTT_CLIENT_ID,
            status
        );


    if (
        len <= 0 ||
        len >= (int)sizeof(payload)
    )
    {
        return;
    }


    esp_mqtt_client_publish(
        s_mqtt_client,
        MQTT_TOPIC_STATUS,
        payload,
        len,
        1,
        0
    );
}


/*
 * ============================================================
 * SENSOR DATA
 * ============================================================
 */

void mqtt_publish_sensor_data(
    const sensor_data_t *data)
{
    if (
        !s_mqtt_connected ||
        data == NULL
    )
    {
        return;
    }


    char payload[320];


    int len = 0;


    /*
     * MMA845x
     */

    if (
        data->type ==
        SENSOR_TYPE_MMA845X
    )
    {
        len =
            snprintf(
                payload,
                sizeof(payload),

                "{"
                "\"device\":\"%s\","
                "\"sensor\":\"mma845x\","
                "\"timestamp_us\":%lld,"
                "\"x_g\":%.5f,"
                "\"y_g\":%.5f,"
                "\"z_g\":%.5f"
                "}",

                MQTT_CLIENT_ID,

                data->timestamp_us,

                data->mma845x.x_g,

                data->mma845x.y_g,

                data->mma845x.z_g
            );
    }


    /*
     * VL53L0X
     */

    else if (
        data->type ==
        SENSOR_TYPE_VL53L0X
    )
    {
        len =
            snprintf(
                payload,
                sizeof(payload),

                "{"
                "\"device\":\"%s\","
                "\"sensor\":\"vl53l0x\","
                "\"timestamp_us\":%lld,"
                "\"distance_mm\":%u,"
                "\"valid\":%s,"
                "\"range_status\":%u"
                "}",

                MQTT_CLIENT_ID,

                data->timestamp_us,

                data->vl53l0x.distance_mm,

                data->vl53l0x.valid
                    ? "true"
                    : "false",

                data->vl53l0x.range_status
            );
    }


    /*
     * Invalid payload
     */

    if (
        len <= 0 ||
        len >= (int)sizeof(payload)
    )
    {
        return;
    }


    /*
     * Publish
     */

    esp_mqtt_client_publish(
        s_mqtt_client,

        MQTT_TOPIC_DATA,

        payload,

        len,

        1,

        0
    );
}