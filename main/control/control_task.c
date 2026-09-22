#include "control_task.h"

#include "app_state.h"
#include "mqtt_manager.h"
#include "config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"


static const char *TAG = "CONTROL";


static QueueHandle_t s_queue = NULL;


/* =========================
 * CONTROL COMMAND
 * ========================= */

typedef enum
{
    CONTROL_CMD_START = 0,

    CONTROL_CMD_STOP

} control_command_t;


/* =========================
 * CONTROL TASK
 * ========================= */

static void control_task(
    void *arg)
{
    control_command_t command;


    while (true)
    {
        /*
         * Chờ command
         */

        if (xQueueReceive(
                s_queue,
                &command,
                portMAX_DELAY
            ) != pdTRUE)
        {
            continue;
        }


        switch (command)
        {
            /* =====================
             * START
             * ===================== */

            case CONTROL_CMD_START:

                ESP_LOGI(
                    TAG,
                    "START"
                );


                app_state_set_running(
                    true
                );


                mqtt_publish_status(
                    "started"
                );

                break;


            /* =====================
             * STOP
             * ===================== */

            case CONTROL_CMD_STOP:

                ESP_LOGI(
                    TAG,
                    "STOP"
                );


                app_state_set_running(
                    false
                );


                mqtt_publish_status(
                    "stopped"
                );

                break;


            default:

                break;
        }
    }
}


/* =========================
 * MQTT CALLBACK
 * ========================= */

static void mqtt_command_callback(
    mqtt_command_t command)
{
    control_command_t
        control_command;


    switch (command)
    {
        case MQTT_COMMAND_START:

            control_command =
                CONTROL_CMD_START;

            break;


        case MQTT_COMMAND_STOP:

            control_command =
                CONTROL_CMD_STOP;

            break;


        default:

            return;
    }


    if (s_queue)
    {
        xQueueSend(
            s_queue,
            &control_command,
            0
        );
    }
}


/* =========================
 * INIT
 * ========================= */

esp_err_t control_task_init(void)
{
    /*
     * Create queue
     */

    s_queue = xQueueCreate(
        COMMAND_QUEUE_LENGTH,
        sizeof(control_command_t)
    );


    if (s_queue == NULL)
    {
        return ESP_ERR_NO_MEM;
    }


    /*
     * Register MQTT callback
     */

    mqtt_manager_set_command_callback(
        mqtt_command_callback
    );


    /*
     * Create task
     */

    BaseType_t ret =
        xTaskCreate(
            control_task,
            "control",
            CONTROL_TASK_STACK,
            NULL,
            CONTROL_TASK_PRIORITY,
            NULL
        );


    if (ret != pdPASS)
    {
        return ESP_FAIL;
    }


    return ESP_OK;
}