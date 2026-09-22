#pragma once

#include <stdbool.h>
#include "esp_err.h"


esp_err_t app_state_init(void);


void app_state_set_running(
    bool running
);


bool app_state_is_running(void);