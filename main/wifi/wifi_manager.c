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


#define WIFI_CONNECTED_BIT    BIT0


static EventGroupHandle_t s_wifi_event_group = NULL;


static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    /*
     * ============================================
     * WIFI START
     * ============================================
     */

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(
            TAG,
            "WiFi STA started"
        );

        esp_wifi_connect();

        return;
    }


    /*
     * ============================================
     * WIFI DISCONNECTED
     * ============================================
     */

    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(
            TAG,
            "WiFi disconnected"
        );


        xEventGroupClearBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );


        /*
         * Reconnect
         */

        esp_wifi_connect();

        return;
    }


    /*
     * ============================================
     * GOT IP
     * ============================================
     */

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;


        ESP_LOGI(
            TAG,
            "Got IP: " IPSTR,
            IP2STR(
                &event->ip_info.ip
            )
        );


        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );

        return;
    }
}


esp_err_t wifi_manager_init(void)
{
    /*
     * Event group
     */

    s_wifi_event_group =
        xEventGroupCreate();


    if (s_wifi_event_group == NULL)
    {
        return ESP_ERR_NO_MEM;
    }


    /*
     * TCP/IP
     */

    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    /*
     * Default event loop
     */

    esp_err_t err =
        esp_event_loop_create_default();


    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        return err;
    }


    /*
     * Default STA
     */

    esp_netif_create_default_wifi_sta();


    /*
     * WiFi driver
     */

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /*
     * Event handlers
     */

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );


    /*
     * WiFi configuration
     */

    wifi_config_t wifi_config = {0};


    strlcpy(
        (char *)wifi_config.sta.ssid,
        WIFI_SSID,
        sizeof(
            wifi_config.sta.ssid
        )
    );


    strlcpy(
        (char *)wifi_config.sta.password,
        WIFI_PASSWORD,
        sizeof(
            wifi_config.sta.password
        )
    );


    /*
     * Authentication
     */

    if (WIFI_PASSWORD[0] == '\0')
    {
        wifi_config.sta.threshold.authmode =
            WIFI_AUTH_OPEN;
    }
    else
    {
        wifi_config.sta.threshold.authmode =
            WIFI_AUTH_WPA2_PSK;
    }


    /*
     * Mode
     */

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA
        )
    );


    /*
     * Config
     */

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );


    /*
     * Start
     */

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(
        TAG,
        "WiFi initialized"
    );


    return ESP_OK;
}


bool wifi_manager_is_connected(void)
{
    if (s_wifi_event_group == NULL)
    {
        return false;
    }


    EventBits_t bits =
        xEventGroupGetBits(
            s_wifi_event_group
        );


    return (
        bits &
        WIFI_CONNECTED_BIT
    ) != 0;
}