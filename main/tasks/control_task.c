#include "control_task.h"

#include "app_state.h"

#include "mqtt_manager.h"

#include "config.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG =
    "CONTROL";


static QueueHandle_t
s_command_queue = NULL;


/*
 * ============================================================
 * CONTROL TASK
 * ============================================================
 */

static void control_task(
    void *arg)
{
    app_command_t command;


    while (true)
    {
        /*
         * Chờ command
         */

        if (
            xQueueReceive(
                s_command_queue,

                &command,

                portMAX_DELAY
            )
            != pdTRUE
        )
        {
            continue;
        }


        /*
         * Process
         */

        switch (command.type)
        {
            /*
             * START
             */

            case APP_CMD_START:
            {
                ESP_LOGI(
                    TAG,

                    "START received"
                );


                /*
                 * Set running bit
                 */

                xEventGroupSetBits(
                    g_app_event_group,

                    APP_RUNNING_BIT
                );


                /*
                 * MQTT status
                 */

                mqtt_publish_status(
                    "started"
                );


                break;
            }


            /*
             * STOP
             */

            case APP_CMD_STOP:
            {
                ESP_LOGI(
                    TAG,

                    "STOP received"
                );


                /*
                 * Clear running bit
                 */

                xEventGroupClearBits(
                    g_app_event_group,

                    APP_RUNNING_BIT
                );


                /*
                 * MQTT status
                 */

                mqtt_publish_status(
                    "stopped"
                );


                break;
            }


            default:
                break;
        }
    }
}


/*
 * ============================================================
 * START TASK
 * ============================================================
 */

void control_task_start(
    QueueHandle_t command_queue)
{
    s_command_queue =
        command_queue;


    xTaskCreate(
        control_task,

        "control_task",

        CONTROL_TASK_STACK,

        NULL,

        CONTROL_TASK_PRIORITY,

        NULL
    );
}