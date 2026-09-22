idf_component_register(
    SRCS
        "main.c"

        "app/app_manager.c"
        "app/app_state.c"

        "communication/wifi_manager.c"
        "communication/mqtt_manager.c"

        "control/control_task.c"

        "sensors/sensor_manager.c"

        "sensors/mma845x/mma845x.c"

        "sensors/vl53l0x/vl53l0x_sensor.c"

    INCLUDE_DIRS
        "."
        "app"
        "communication"
        "control"
        "sensors"
        "sensors/mma845x"
        "sensors/vl53l0x"

    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        mqtt
        nvs_flash
        driver
)