/**
 ****************************************************************************************
 *
 * @file bt_common.h
 *
 * @brief Classic Bluetooth Common Header
 *
 ****************************************************************************************
 * @attention
  #####Copyright (c) 2023 GOODIX
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

#ifndef __BT_COMMON_H__
#define __BT_COMMON_H__

// UART
#define BT_IFCE_UART_ID              APP_UART_ID_1
#define BT_IFCE_UART_BAUDRATE_NORMAL 2000000
#define BT_IFCE_UART_DATABITS        UART_DATABITS_8
#define BT_IFCE_UART_STOPBITS        UART_STOPBITS_1
#define BT_IFCE_UART_PARITY          UART_PARITY_NONE
#define BT_IFCE_UART_HW_FLOW_CTRL    UART_HWCONTROL_NONE
#define BT_IFCE_UART_RX_TIMEOUT      UART_RECEIVER_TIMEOUT_ENABLE

#define BT_IFCE_UART_RX_IO_TYPE      APP_IO_TYPE_GPIOB
#define BT_IFCE_UART_RX_IO_PIN       APP_IO_PIN_10
#define BT_IFCE_UART_RX_PULL         APP_IO_PULLUP
#define BT_IFCE_UART_RX_IO_MUX       APP_IO_MUX_1

#define BT_IFCE_UART_TX_IO_TYPE      APP_IO_TYPE_GPIOB
#define BT_IFCE_UART_TX_IO_PIN       APP_IO_PIN_14
#define BT_IFCE_UART_TX_PULL         APP_IO_NOPULL
#define BT_IFCE_UART_TX_IO_MUX       APP_IO_MUX_1

#define BT_IFCE_UART_TX_MAX_LEN      1024
#define BT_IFCE_UART_RX_MAX_LEN      1024

#define BT_IFCE_UART_USE_TX_DMA      0
#define BT_IFCE_UART_USE_RX_DMA      0

#define BT_IFCE_UART_DMA_INST        DMA0
#define BT_IFCE_UART_TX_DMA_CHANNEL  DMA_Channel2
#define BT_IFCE_UART_RX_DMA_CHANNEL  DMA_Channel3

// BT -> BLE Message Indication
#define BT_IFCE_IND_IO_TYPE APP_IO_TYPE_AON
#define BT_IFCE_IND_IO_PIN  APP_IO_PIN_7
#define BT_IFCE_IND_IO_MODE APP_IO_MODE_IT_RISING
#define BT_IFCE_IND_IO_PULL APP_IO_PULLDOWN
#define BT_IFCE_IND_IO_MUX  APP_IO_MUX

// BLE -> BT Synchronization
#define BT_IFCE_SYNC_IO_TYPE APP_IO_TYPE_GPIOB
#define BT_IFCE_SYNC_IO_PIN  APP_IO_PIN_11
#define BT_IFCE_SYNC_IO_MODE APP_IO_MODE_OUTPUT
#define BT_IFCE_SYNC_IO_PULL APP_IO_NOPULL
#define BT_IFCE_SYNC_IO_MUX  APP_IO_MUX

// BT Reset
#define BT_IFCE_RESET_IO_TYPE APP_IO_TYPE_GPIOB
#define BT_IFCE_RESET_IO_PIN  APP_IO_PIN_12
#define BT_IFCE_RESET_IO_MODE APP_IO_MODE_OUTPUT
#define BT_IFCE_RESET_IO_PULL APP_IO_NOPULL
#define BT_IFCE_RESET_IO_MUX  APP_IO_MUX

// Message Frame
#define BT_DATA_HEAD_CMD 0xABBA
#define BT_DATA_HEAD_ACK 0xAABB
#define BT_DATA_HEAD_IND 0xCDDC

// Command Opcode
typedef enum {
    /**
     * Audio
     */
    OPCODE_A2DP_STATUS_SET,
    OPCODE_PLAY_STATUS_SET,
    OPCODE_PLAY_NEXT,
    OPCODE_PLAY_PREV,
    OPCODE_PLAY_REWIND,
    OPCODE_PLAY_FORWARD,
    OPCODE_AUDIO_VOL_SET,
    OPCODE_AUDIO_VOL_GET,
    OPCODE_VOL_GAIN_SET,
    OPCODE_VOL_GAIN_GET,
    OPCODE_AUDIO_INFO_GET,
    OPCODE_AUDIO_WORDS_GET,

    /**
     * Phone Call
     */
    OPCODE_CALL_ADDR_BOOK_GET,
    OPCODE_CALL_ACCEPT,
    OPCODE_CALL_DTMF_SET,
    OPCODE_CALL_END,
    OPCODE_CALL_REJECT,
    OPCODE_CALL_REDIAL,
    OPCODE_CALL_VOL_SET,
    OPCODE_CALL_VOL_GET,
    OPCODE_CALL_VOL_GAIN_SET,
    OPCODE_CALL_VOL_GAIN_GET,
    OPCODE_CALL_VOL_SWITCH,
    OPCODE_CALL_HOLD_FIR_ACCEPT_SEC,
    OPCODE_CALL_END_FIR_ACCEPT_SEC,
    OPCODE_CALL_HOLD_FIR_REJECT_SEC,
    OPCODE_CALL_LINE_SWITCH,
    OPCODE_CALL_NUMBER_GET,
    OPCODE_CALL_DIAL_NUMBER,
    OPCODE_CALL_RING_STATE_SET,
    OPCODE_CALL_RING_STATE_GET,

    /**
     * System Control
     */
    OPCODE_BT_RESET,
    OPCODE_BT_INQUIRY,
    OPCODE_BT_INQUIRY_SCAN,
    OPCODE_BT_CONNECT,
    OPCODE_BT_CONNECT_ABORT,
    OPCODE_BT_PAGE_SCAN,
    OPCODE_BT_PAIRING_BEGIN,
    OPCODE_BT_QUICK_CONNECT,
    OPCODE_BT_DISCONNECT,
    OPCODE_BT_SWITCH_PHONE,
    OPCODE_BT_PAIRING_INFO_STORE,
    OPCODE_BT_PAIRING_INFO_CLEAN,
    OPCODE_BT_ACTIVE_TIMEOUT_SET,
    OPCODE_BT_SNIFF_INTERVAL_SET,
    OPCODE_BT_QUICK_CON_TIMES,
    OPCODE_BT_QUICK_CON_MODE_SET,
    OPCODE_BT_BD_ADDR_GET,
    OPCODE_BT_DEVICE_NAME_SET,
    OPCODE_BT_DEVICE_NAME_GET,

    OPCODE_BT_THRPUT_BEGIN,
    OPCODE_BT_THRPUT_DATA,
    OPCODE_BT_THRPUT_END,

    OPCODE_BT_FW_VERSION_GET,
    OPCODE_BT_OTA_BEGIN,

    OPCODE_BT_SIRI_WAKE,
    OPCODE_BT_SWITCH_NOTICE,
    OPCODE_BT_MIC_MUTE,
    OPCODE_VOL_P,
    OPCODE_VOL_M,
    OPCODE_VOL_MUTE,
    OPCODE_BT_MIC_GAIN_SET,
    OPCODE_BT_MIC_GAIN_GET,
    OPCODE_BT_MIC_TEST_START,
    OPCODE_BT_MIC_TEST_STOP,
    OPCODE_BT_MIC_AUTO_TEST,
    OPCODE_BT_CUSTOM_NOTICE_PLAY,
    OPCODE_BT_CUSTOM_NOTICE_PAUSE,

    OPCODE_BT_UART_MAX_LENGTH,
    OPCODE_BT_FACTORY_RESET,
    OPCODE_BT_POWERDOWN,
    OPCODE_BT_WAKEUP,
    OPCODE_BT_PAIR_DEV_LIST_GET,
    OPCODE_BT_CALL_OFFSET_SET,
    OPCODE_BT_CALL_OFFSET_GET,
    OPCODE_BT_LOG_CTRL,
    OPCODE_BT_BD_ADDR_SET,
    OPCODE_BT_INQ_PAGE_GET,
    OPCODE_BT_CONN_STATE_GET,

    OPCODE_BT_BATT_LV_SYNC,

    OPCODE_CUR_VOL_GET,
    OPCODE_CUR_GAIN_GET,
    OPCODE_VOL_MUTE_GET,
    OPCODE_MIC_MUTE_GET,

    OPCODE_DRAIN_VIBRA_START,
    OPCODE_DRAIN_VIBRA_STOP,
    OPCODE_SEARCH_DEV_START,
    OPCODE_SEARCH_DEV_STOP,

    OPCODE_BT_SIRI_SHUTDOWN,
    OPCODE_BT_SIRI_STATE_GET,

    OPCODE_BT_NOTICE_LVL_SET,
    OPCODE_BT_NOTICE_LVL_GET,

    OPCODE_BT_MAX,

    OPCODE_BT_TEST_CMD = 0xFF,
} bt_opcode_t;

typedef enum {
    RESP_BT_NO_ERR,
    RESP_BT_INVALID_PARAM,
    RESP_BT_A2DP_NO_CONN_STATE,
    RESP_BT_HFP_NO_CONN_STATE,
    RESP_BT_NO_CALL_LINE,
    RESP_BT_NO_CONN_STATE,
    RESP_BT_PHONE_BOOK_ACCESS_DENIED,
    RESP_BT_ADDRESS_INEXISTENCE,
    RESP_BT_OP_CONDITION_NOT_MET,
    RESP_BT_IS_CONNECTTED,
    RESP_BT_NO_PAIR_RECORD,
    RESP_BT_EXIST_CALL_LINE,
    RESP_BT_ERR_CPDE_MAX,
} bt_resp_t;

typedef enum {
    IND_PAIRING_INFO, /* Deprecated */
    IND_PROFILE_STATE,
    IND_INQUIRY_INFO,
    IND_CONNECTION_STATE,
    IND_BT_BD_ADDR,
    IND_BT_DEV_NAME,
    IND_THROUGHPUT_CMP, /* Deprecated */
    IND_FW_VERSION_INFO,
    IND_VOL_LEVEL_INFO,
    IND_MIC_GAIN_INFO,
    IND_ADDR_BOOK_INFO,
    IND_CALL_LINE_STATE,
    IND_CALL_RING_STATE,
    IND_VOL_SITE_INFO,
    IND_AUDIO_SONG_INFO,
    IND_AUDIO_SONG_WORD_INFO,
    IND_AUDIO_STATE,
    IND_POWER_ON_STATE,
    IND_PAIR_DEV_INFO,
    IND_BT_CALL_OFFSET,
    IND_BT_LOG_STATE,
    IND_BT_INQ_PAGE_STATE,
    IND_BT_SNIFF_STATE,

    IND_HFP_VOL_INFO,
    IND_AUDIO_VOL_INFO,
    IND_HFP_GAIN_INFO,
    IND_AUDIO_GAIN_INFO,
    IND_VOL_MUTE_INFO,
    IND_MIC_MUTE_INFO,

    IND_LOCAL_VOICE_STATE,
    IND_SIRI_STATE,

    IND_NOTICE_LVL_STATE,

    IND_MESSAGE_MAX,

    IND_BT_TEST_MSG = 0xFF,
} bt_ind_type_t;

#define BT_SYNC_WAKEUP_PULSE_WIDTH_FROM_ULTRASLEEP 20
#define BT_SYNC_WAKEUP_PULSE_WIDTH_FROM_SNIFF 3

#endif // __BT_COMMON_H__
