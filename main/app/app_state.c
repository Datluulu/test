#include "app_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define APP_RUNNING_BIT BIT0

static EventGroupHandle_t s_event_group = NULL;


esp_err_t app_state_init(void)
{
    s_event_group = xEventGroupCreate();

    if (s_event_group == NULL)
        return ESP_ERR_NO_MEM;

    return ESP_OK;
}


void app_state_set_running(bool running)
{
    if (s_event_group == NULL)
        return;

    if (running)
    {
        xEventGroupSetBits(
            s_event_group,
            APP_RUNNING_BIT
        );
    }
    else
    {
        xEventGroupClearBits(
            s_event_group,
            APP_RUNNING_BIT
        );
    }
}


bool app_state_is_running(void)
{
    if (s_event_group == NULL)
        return false;

    return (
        xEventGroupGetBits(s_event_group)
        & APP_RUNNING_BIT
    ) != 0;
}