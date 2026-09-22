#include "wifi_manager.h"

#include "config.h"

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <string.h>


static const char *TAG = "WIFI";

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t s_wifi_event_group = NULL;


static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WiFi started");

        esp_wifi_connect();

        return;
    }


    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG, "WiFi disconnected");

        xEventGroupClearBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );

        esp_wifi_connect();

        return;
    }


    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = event_data;

        ESP_LOGI(
            TAG,
            "Got IP: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}


esp_err_t wifi_manager_init(void)
{
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL)
        return ESP_ERR_NO_MEM;


    esp_err_t err = esp_netif_init();

    if (err != ESP_OK)
        return err;


    err = esp_event_loop_create_default();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        return err;
    }


    esp_netif_create_default_wifi_sta();


    wifi_init_config_t wifi_init =
        WIFI_INIT_CONFIG_DEFAULT();


    err = esp_wifi_init(&wifi_init);

    if (err != ESP_OK)
        return err;


    err = esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        wifi_event_handler,
        NULL
    );

    if (err != ESP_OK)
        return err;


    err = esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        wifi_event_handler,
        NULL
    );

    if (err != ESP_OK)
        return err;


    wifi_config_t config = {0};


    strlcpy(
        (char *)config.sta.ssid,
        WIFI_SSID,
        sizeof(config.sta.ssid)
    );


    strlcpy(
        (char *)config.sta.password,
        WIFI_PASSWORD,
        sizeof(config.sta.password)
    );


    if (WIFI_PASSWORD[0] == '\0')
    {
        config.sta.threshold.authmode =
            WIFI_AUTH_OPEN;
    }
    else
    {
        config.sta.threshold.authmode =
            WIFI_AUTH_WPA2_PSK;
    }


    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );


    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &config
        )
    );


    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(TAG, "WiFi initialized");

    return ESP_OK;
}


bool wifi_manager_is_connected(void)
{
    if (s_wifi_event_group == NULL)
        return false;

    return (
        xEventGroupGetBits(
            s_wifi_event_group
        )
        & WIFI_CONNECTED_BIT
    ) != 0;
}