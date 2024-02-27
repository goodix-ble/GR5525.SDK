/**
 *****************************************************************************************
 *
 * @file bt_cmd_test.c
 *
 * @brief bt cmd test function Implementation.
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
#include "utility.h"
#include "app_timer.h"
#include "app_log.h"
#include "app_error.h"
#include "bt_cmd_test.h"
#include "bt_ctl_comm_interface.h"
#include "bt_ctl_call_interface.h"
#include "bt_ctl_audio_interface.h"

#ifdef BLE_BT_OTA_SUPPORT
extern void app_bt_ota_procedure_begin(uint32_t flash_qspi_addr, uint32_t flash_fw_size);
#endif

extern void ble_sec_get_link_key(uint8_t idx, uint8_t* peer_addr, uint8_t*link_key);

void app_bt_call_place_test(uint8_t *cmd, uint16_t length)
{
    uint16_t idx = 0;
    uint8_t call_num_arr[16] = {'0', '2', '8', '6', '5', '1', '9', '8', '7', '6', '0'};

    if (length <= 16)
    {
        for (idx = 0; idx < length; idx ++)
        {
            call_num_arr[idx] = cmd[idx] + '0';
        }

        app_bt_place_call_number(call_num_arr, length);
    }
}

void app_bt_call_vol_gain_test(uint8_t *cmd, uint16_t length)
{
    app_bt_call_vol_gain_set(cmd[0], cmd[1], cmd[2]);
}

void app_bt_vol_gain_test(uint8_t *cmd, uint16_t length)
{
    app_bt_vol_gain_set(cmd[0], cmd[1], cmd[2]);
}

void app_bt_call_vol_test(uint8_t *cmd, uint16_t length)
{
    app_bt_call_vol_level_set(cmd[0]);
}

void app_bt_log_ctrl_set_test(uint8_t *cmd, uint16_t length)
{
    app_bt_log_ctrl_set(cmd[0]);
}

extern void app_bt_batt_lvl_info_sync(uint8_t lvl);
void app_bt_batt_level_test(uint8_t *cmd, uint16_t length)
{
    app_bt_batt_lvl_info_sync(cmd[0]);
}

void app_bt_vol_test(uint8_t *cmd, uint16_t length)
{
    app_bt_vol_level_set(cmd[0]);
}

void app_bt_mic_gain_set_test(uint8_t *cmd, uint16_t length)
{
    app_bt_mic_gain_set(cmd[0], cmd[1]);
}

void app_bt_vol_mute_test(uint8_t *cmd, uint16_t length)
{
    app_bt_vol_mute(cmd[0]);
}

void app_bt_mic_mute_test(uint8_t *cmd, uint16_t length)
{
    app_bt_mic_mute(cmd[0]);
}

void app_bt_device_name_set_test(uint8_t *cmd, uint16_t length)
{
#if 0
    if(length < 32)
    {
        app_bt_device_name_set(cmd, length);
    }
#else
    uint8_t device_name[]  = {'g', 'd', 'x', '_', 'd', 'u', 'a', 'l', '_', 'a', 'p','p'};

    app_bt_device_name_set(device_name, sizeof(device_name));
#endif
}

void app_bt_device_addr_set_test(uint8_t *cmd, uint16_t length)
{
#if 0
    if(length == 6)
    {
        app_bt_device_name_set(cmd, length);
    }
#else
    uint8_t device_addr[]  = {0xaf, 0xbb, 0xcf, 0x3e, 0xcb, 0xea};

    app_bt_addr_set(device_addr);
#endif
}

void app_bt_pairing_info_clean_test(uint8_t *cmd, uint16_t length)
{
    app_bt_pairing_info_clean(cmd);
}

void app_bt_call_dtmf_test(uint8_t *cmd, uint16_t length)
{
    app_bt_call_dtmf_set(cmd, length);
}


void app_bt_enable_quick_connect_test(uint8_t *cmd, uint16_t length)
{
    static bool flag = false;
    app_bt_enable_quick_connect(cmd[0]);
    flag = !flag;
}

void app_bt_ota_param_update_test(void)
{
#ifdef BLE_BT_OTA_SUPPORT
    // 1M FW = 0xFF000, 512K FW=0x7F000
    app_bt_ota_procedure_begin( FLASH_PROGRAM_START_ADDR, 0x7F000);
#endif
}

uint8_t delay_more = 1;
void app_wakeup_bt_test(uint8_t *cmd, uint16_t length)
{
    extern void ble_bt_reset_sync_io(void);
    extern void ble_bt_set_sync_io(void);

    ble_bt_reset_sync_io();
    delay_ms(cmd[0]);
    ble_bt_set_sync_io();
    delay_more = cmd[0];
    APP_LOG_DEBUG("app_bt_wakeup_bt, OUTPUT: LOW %d ", delay_more);
}

void app_bt_notice_level_set_test(uint8_t *cmd, uint16_t length)
{
    app_bt_notice_level_set(cmd[0]);
}


func_ptr_t  func_arr[CMD_ARR_MAX]=
{
    &app_bt_device_reset,//0x00
    &app_bt_device_factory_reset,//0x01
    &app_bt_wakeup_bt,//0x02
    &app_bt_device_inquiry_scan_start,//0x03
    &app_bt_device_inquiry_scan_stop,//0x04
    &app_bt_connect_abort,//0x05
    &app_bt_device_page_scan_start,//0x06
    &app_bt_device_page_scan_stop,//0x07
    &app_bt_quick_reconnect,//0x08
    &app_bt_disconnect,//0x09
    &app_bt_addr_get,//0x0A
    &app_bt_device_name_get,//0x0B
    &app_bt_software_version_get,//0x0C
    &app_bt_siri_wake_up,//0x0D
    &app_bt_volp_action,//0x0E
    &app_bt_volm_action,//0x0F
    &app_bt_mic_gain_get,//0x10
    &app_bt_mic_test_start,//0x11
    &app_bt_mic_test_stop,//0x12
    &app_bt_cur_vol_get,// 0x13

    &app_bt_calling_number_get,//0x14
    &app_bt_call_address_book_get,//0x15
    &app_bt_call_accept,//0x16
    &app_bt_call_end,//0x17
    &app_bt_call_reject,//0x18
    &app_bt_call_redial,//0x19
    &app_bt_call_vol_level_get,//0x1a
    &app_bt_call_vol_gain_get,//0x1b
    &app_bt_call_vol_switch,//0x1c
    &app_bt_three_way_call_switch,//0x1d

    &app_bt_audio_info_get,//0x1e
    &app_bt_a2dp_status_set,//0x1f
    &app_bt_avrcp_status_set,//0x20
    &app_bt_avrcp_play_next,//0x21
    &app_bt_avrcp_play_prev,//0x22
    &app_bt_avrcp_play_rewind,//0x23
    &app_bt_avrcp_play_forward,//0x24
    &app_bt_vol_level_get,//0x25
    &app_bt_vol_gain_get,//0x26

    &app_bt_vol_mute_get,// 0x27
    &app_bt_mic_mute_get,// 0x28
    &app_bt_pair_list_get,//0x29
    &app_bt_conn_state_get,//0x2A
    &app_bt_mic_gain_get,//0x2B
    &app_bt_reset_hw,//0x2C
    &app_bt_powerdown_bt,//0x2D
    &app_bt_ota_param_update_test,//0x2E

    &app_bt_drain_vibra_start,// 0x2F
    &app_bt_drain_vibra_stop,// 0x30
    &app_bt_search_dev_start,// 0x31
    &app_bt_search_dev_stop,// 0x32
    &app_bt_siri_exit,//0x33
    &app_bt_siri_state_get,//0x34
    &app_bt_notice_level_get,//0x35
};

func_param_ptr_t  func_param_arr[CMD_ARR_MAX] =
{
    /*eg. HEX: 80 01 00 00 08 06*/
    &app_bt_call_place_test,//0x80
    /*eg.HEX: 81 0F 2B */
    &app_bt_call_vol_gain_test,//0x81
    /*eg.HEX: 82 0F 2B */
    &app_bt_vol_gain_test,//0x82
    &app_bt_call_vol_test,//0x83
    &app_bt_vol_test,//0x84
    &app_bt_call_dtmf_test,//0x85
    &app_bt_mic_gain_set_test,//0x86
    &app_bt_vol_mute_test,//0x87
    &app_bt_mic_mute_test,//0x88
    &app_bt_device_name_set_test,//0x89
    &app_bt_device_addr_set_test,//0x8a
    &app_bt_pairing_info_clean_test,//0x8b
    &app_bt_enable_quick_connect_test,//0x8c
    &app_bt_log_ctrl_set_test,//0x8d
    &app_bt_batt_level_test,//0x8e
    &app_wakeup_bt_test,//0x8f
    &app_bt_notice_level_set_test,//0x90
};

