#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define APP_RUNNING_BIT    BIT0


extern EventGroupHandle_t g_app_event_group;