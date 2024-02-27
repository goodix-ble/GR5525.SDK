/**
 *****************************************************************************************
 *
 * @file main.c
 *
 * @brief main function Implementation.
 *
 *****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "user_app.h"
#include "user_periph_setup.h"
#include "gr_includes.h"
#include "scatter_common.h"
#include "patch.h"
#include "app_log.h"
#include "app_uart.h"
#include "bt_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bt_ctl_debug_interface.h"

/**
 * @brief Entry task to create other tasks
 *
 * @param[in] arg Unused
 */
static void vStartTasks(void *arg)
{
    extern void app_create_bt_task(void);
    app_create_bt_task();

    vTaskDelay(2000);
    APP_LOG_DEBUG("Set Name!!");
    bt_api_device_name_set("BTv2_API_TEST", strlen("BTv2_API_TEST"));
    APP_LOG_DEBUG("Get Name!!");
    bt_api_device_name_get();
    vTaskDelete(NULL);
}

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
/**@brief Stack global variables for Bluetooth protocol stack. */
STACK_HEAP_INIT(heaps_table);
int main(void)
{
    // Initialize user peripherals.
    app_periph_init();

    // Initialize BT API
    bt_api_init();

    // Initialize BLE Stack.
    ble_stack_init(ble_evt_handler, &heaps_table);

    xTaskCreate(vStartTasks, "create_task", 1024, NULL, 0, NULL);
    vTaskStartScheduler();
    while (1)
        ;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf(">>>> FReeRTOS Task %s Overflow  !!!\r\n", pcTaskName);
}

void vApplicationMallocFailedHook()
{
    printf(">>>> FReeRTOS Malloc FAILED !!!\r\n");
}

void vApplicationIdleHook()
{
    bt_debug_task();
}
