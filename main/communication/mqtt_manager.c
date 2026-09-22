#include "mqtt_manager.h"

#include "config.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include <string.h>
#include <stdio.h>


static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t s_client = NULL;

static volatile bool s_connected = false;

static mqtt_command_callback_t s_command_callback = NULL;


void mqtt_manager_set_command_callback(
    mqtt_command_callback_t callback)
{
    s_command_callback = callback;
}


static void process_command(
    const char *topic,
    int topic_len)
{
    if (topic == NULL)
        return;


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

        ESP_LOGI(TAG, "START received");

        return;
    }


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

        ESP_LOGI(TAG, "STOP received");

        return;
    }
}


static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        event_data;


    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:

            s_connected = true;

            ESP_LOGI(
                TAG,
                "MQTT connected"
            );


            esp_mqtt_client_subscribe(
                s_client,
                MQTT_TOPIC_START,
                1
            );


            esp_mqtt_client_subscribe(
                s_client,
                MQTT_TOPIC_STOP,
                1
            );


            mqtt_publish_status(
                "online"
            );

            break;


        case MQTT_EVENT_DISCONNECTED:

            s_connected = false;

            ESP_LOGW(
                TAG,
                "MQTT disconnected"
            );

            break;


        case MQTT_EVENT_DATA:

            process_command(
                event->topic,
                event->topic_len
            );

            break;


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


esp_err_t mqtt_manager_init(void)
{
    esp_mqtt_client_config_t config = {
        .broker.address.uri =
            MQTT_BROKER_URI,
    };


    if (MQTT_USERNAME[0] != '\0')
    {
        config.credentials.username =
            MQTT_USERNAME;
    }


    if (MQTT_PASSWORD[0] != '\0')
    {
        config.credentials.authentication.password =
            MQTT_PASSWORD;
    }


    if (MQTT_CLIENT_ID[0] != '\0')
    {
        config.credentials.client_id =
            MQTT_CLIENT_ID;
    }


    s_client =
        esp_mqtt_client_init(&config);


    if (s_client == NULL)
    {
        ESP_LOGE(
            TAG,
            "MQTT client init failed"
        );

        return ESP_FAIL;
    }


    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            s_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_mqtt_client_start(s_client)
    );


    ESP_LOGI(
        TAG,
        "MQTT started: %s",
        MQTT_BROKER_URI
    );


    return ESP_OK;
}


bool mqtt_manager_is_connected(void)
{
    return s_connected;
}


void mqtt_publish_status(
    const char *status)
{
    if (!s_connected || status == NULL)
        return;


    char payload[160];


    int len = snprintf(
        payload,
        sizeof(payload),
        "{\"device\":\"%s\",\"status\":\"%s\"}",
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


void mqtt_publish_sensor_data(
    const sensor_data_t *data)
{
    if (!s_connected || data == NULL)
        return;


    char payload[320];

    int len = 0;


    if (data->type == SENSOR_TYPE_MMA845X)
    {
        len = snprintf(
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
    else if (data->type == SENSOR_TYPE_VL53L0X)
    {
        len = snprintf(
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


    if (len <= 0 ||
        len >= sizeof(payload))
    {
        return;
    }


    esp_mqtt_client_publish(
        s_client,
        MQTT_TOPIC_DATA,
        payload,
        len,
        1,
        0
    );
}