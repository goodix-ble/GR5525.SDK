#include "FreeRTOS.h"
#include "task.h"
#include "bt_api.h"
#include "bt_ifce.h"
#include "bt_ctl_debug_interface.h"

#include <stdio.h>

#define BT_TASK_BIT_DATA_AVIALABLE (1 << 0)

static TaskHandle_t s_bt_task_handle;

static void app_bt_task(void *args);

void app_create_bt_task(void)
{
    xTaskCreate(app_bt_task, "app_bt_task", 2048, NULL, configMAX_PRIORITIES - 1, &s_bt_task_handle);
}

void bt_ifce_data_available(void)
{
    if (__get_IPSR())
    {
        BaseType_t xHigherPriorityTaskWoken;
        xTaskNotifyFromISR(s_bt_task_handle, BT_TASK_BIT_DATA_AVIALABLE, eSetBits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        xTaskNotify(s_bt_task_handle, BT_TASK_BIT_DATA_AVIALABLE, eSetBits);
    }
}

static void app_bt_task(void *args)
{
    uint32_t flags = 0;
    while(1)
    {
        if (flags & BT_TASK_BIT_DATA_AVIALABLE)
        {
            flags &= ~BT_TASK_BIT_DATA_AVIALABLE;
            bt_api_msg_handler();
        }
        xTaskNotifyWait(0, 0xFFFFFFFF, &flags, portMAX_DELAY);
    }
}

