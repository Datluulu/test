idf_component_register(
    SRCS
        "app_main.c"

        "wifi/wifi_manager.c"

        "mqtt/mqtt_manager.c"

        "sensors/mma845x.c"
        "sensors/vl53l0x_sensor.c"

        "tasks/control_task.c"
        "tasks/sensor_tasks.c"

    INCLUDE_DIRS
        "."
        "wifi"
        "mqtt"
        "sensors"
        "tasks"

    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        esp_driver_i2c
        esp_timer
)