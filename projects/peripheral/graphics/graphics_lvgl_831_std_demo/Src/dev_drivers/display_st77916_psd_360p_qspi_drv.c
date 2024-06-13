#include "display_st77916_psd_360p_qspi_drv.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "app_qspi.h"
#include "app_qspi_dma.h"
#include "app_pwm.h"

#if APP_DRIVER_CHIP_TYPE == APP_DRIVER_GR5525X
    #if GR5625_SK
        #define DISPLAY_RESET_IO_TYPE APP_IO_TYPE_GPIOA
        #define DISPLAY_RESET_IO_PIN APP_IO_PIN_13
        #define DISPLAY_RESET_IO_MUX APP_IO_MUX_8

        #define DISPLAY_TE_IO_TYPE APP_IO_TYPE_GPIOA
        #define DISPLAY_TE_IO_PIN APP_IO_PIN_12
        #define DISPLAY_TE_IO_MUX APP_IO_MUX_8

        #define DISPLAY_BL_PWM_ID APP_PWM_ID_1
        #define DISPLAY_BL_IO_TYPE APP_IO_TYPE_MSIO
        #define DISPLAY_BL_IO_PIN APP_IO_PIN_4
        #define DISPLAY_BL_IO_MUX APP_IO_MUX_0
    #else
        #define DISPLAY_RESET_IO_TYPE APP_IO_TYPE_AON
        #define DISPLAY_RESET_IO_PIN APP_IO_PIN_6
        #define DISPLAY_RESET_IO_MUX APP_IO_MUX_8

        #define DISPLAY_TE_IO_TYPE APP_IO_TYPE_AON
        #define DISPLAY_TE_IO_PIN APP_IO_PIN_5
        #define DISPLAY_TE_IO_MUX APP_IO_MUX_8

        #define DISPLAY_BL_PWM_ID APP_PWM_ID_0
        #define DISPLAY_BL_IO_TYPE APP_IO_TYPE_GPIOB
        #define DISPLAY_BL_IO_PIN APP_IO_PIN_15
        #define DISPLAY_BL_IO_MUX APP_IO_MUX_3
    #endif // GR5625_SK
#else
    #error "NOT SUPPORT"
#endif

static app_qspi_params_t s_qspi_param = {
    .id = APP_QSPI_ID_2,
    .pin_cfg = {
        .cs = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_6,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
        .clk = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_3,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
        .io_0 = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_4,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
        .io_1 = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_5,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
        .io_2 = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_2,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
        .io_3 = {
            .type = APP_IO_TYPE_GPIOA,
            .mux = APP_IO_MUX_7,
            .pin = APP_IO_PIN_7,
            .mode = APP_IO_MODE_MUX,
            .pull = APP_IO_NOPULL,
            .enable = APP_QSPI_PIN_ENABLE,
        },
    },
    .dma_cfg = {
        .dma_instance = DMA1,
        .dma_channel = DMA_Channel0,
        .wait_timeout_ms = 3000,
        .extend = 0,
    },
    .init = {
        .clock_prescaler = 2,
        .clock_mode = QSPI_CLOCK_MODE_0,
        .rx_sample_delay = 0,
    },
};

static app_pwm_params_t s_pwm_param = {
    .id = DISPLAY_BL_PWM_ID,
    .pin_cfg = {
        .channel_b = {
            .type = DISPLAY_BL_IO_TYPE,
            .mux = DISPLAY_BL_IO_MUX,
            .pin = DISPLAY_BL_IO_PIN,
            .pull = APP_IO_PULLUP,
            .enable = 1,
        },
        .channel_a = {.enable = 0},
        .channel_c = {.enable = 0},
    },
    .active_channel = APP_PWM_ACTIVE_CHANNEL_B,
    .init = {
        .mode = PWM_MODE_FLICKER,
        .align = PWM_ALIGNED_EDGE,
        .freq = 100*1000,
        .bperiod = 500,
        .hperiod = 200,
        .bstoplvl = PWM_STOP_LVL_LOW,
        .channel_b = {
            .duty = 10,
            .drive_polarity = PWM_DRIVEPOLARITY_POSITIVE,
            .fstoplvl = PWM_STOP_LVL_LOW,
        },
    },
};

static SemaphoreHandle_t s_te_sem = NULL;

static void display_te_evt_callback(app_io_evt_t *p_evt);
static void display_qspi_spi_send_sync(uint32_t cmd, uint8_t *p_data, uint16_t len);
static void st77916_init_sequence(void);

void display_st77916_psd_360p_init(app_qspi_id_t id, uint32_t clock_prescaler, app_qspi_evt_handler_t qspi_evt_handler)
{
    // QSPI
    app_qspi_init(&s_qspi_param, qspi_evt_handler);
    app_qspi_dma_init(&s_qspi_param);

#if APP_DRIVER_CHIP_TYPE == APP_DRIVER_GR5525X
    app_io_set_strength(s_qspi_param.pin_cfg.cs.type,   s_qspi_param.pin_cfg.cs.pin,   APP_IO_STRENGTH_ULTRA);
    app_io_set_strength(s_qspi_param.pin_cfg.clk.type,  s_qspi_param.pin_cfg.clk.pin,  APP_IO_STRENGTH_ULTRA);
    app_io_set_strength(s_qspi_param.pin_cfg.io_0.type, s_qspi_param.pin_cfg.io_0.pin, APP_IO_STRENGTH_ULTRA);
    app_io_set_strength(s_qspi_param.pin_cfg.io_1.type, s_qspi_param.pin_cfg.io_1.pin, APP_IO_STRENGTH_ULTRA);
    app_io_set_strength(s_qspi_param.pin_cfg.io_2.type, s_qspi_param.pin_cfg.io_2.pin, APP_IO_STRENGTH_ULTRA);
    app_io_set_strength(s_qspi_param.pin_cfg.io_3.type, s_qspi_param.pin_cfg.io_3.pin, APP_IO_STRENGTH_ULTRA);
#endif

    // TE Semaphore
    s_te_sem = xSemaphoreCreateBinary();

    // RESET Pin
    app_io_init_t reset_pin_init = {
        .pin = DISPLAY_RESET_IO_PIN,
        .mode = APP_IO_MODE_OUTPUT,
        .pull = APP_IO_NOPULL,
        .mux = DISPLAY_RESET_IO_MUX,
    };
    app_io_init(DISPLAY_RESET_IO_TYPE, &reset_pin_init);
    app_io_write_pin(DISPLAY_RESET_IO_TYPE, DISPLAY_RESET_IO_PIN, APP_IO_PIN_SET);

    // TE Pin
    app_io_init_t te_pin_init = {
        .pin = DISPLAY_TE_IO_PIN,
        .mode = APP_IO_MODE_IT_FALLING,
        .pull = APP_IO_PULLUP,
        .mux = DISPLAY_TE_IO_MUX,
    };
    app_io_event_register_cb(DISPLAY_TE_IO_TYPE, &te_pin_init, display_te_evt_callback, NULL);
#if GR5625_SK
    ll_gpio_disable_it(GPIO0, LL_GPIO_PIN_12);
#else
    ll_aon_gpio_disable_it(LL_AON_GPIO_PIN_5);
#endif // GR5625_SK

    // LEDK Pin
    app_io_init_t ledk_pin_init = {
        .pin = DISPLAY_BL_IO_PIN,
        .mode = APP_IO_MODE_OUTPUT,
        .pull = APP_IO_NOPULL,
        .mux = DISPLAY_BL_IO_MUX,
    };
    app_io_init(DISPLAY_BL_IO_TYPE, &ledk_pin_init);
    app_io_write_pin(DISPLAY_BL_IO_TYPE, DISPLAY_BL_IO_PIN, APP_IO_PIN_SET);
//    uint16_t err = app_pwm_init(&s_pwm_param);
//    printf("app_pwm_init = %d\n", err);
//    err = app_pwm_start(s_pwm_param.id);
//    printf("app_pwm_start = %d\n", err);

    // Reset
    delay_ms(100);
    app_io_write_pin(DISPLAY_RESET_IO_TYPE, DISPLAY_RESET_IO_PIN, APP_IO_PIN_RESET);
    delay_ms(100);
    app_io_write_pin(DISPLAY_RESET_IO_TYPE, DISPLAY_RESET_IO_PIN, APP_IO_PIN_SET);
    delay_ms(200);

    // Read Module ID
    app_qspi_command_t rd_cmd = {
        .instruction = 0x0B,
        .address = 0x000400,
        .instruction_size = QSPI_INSTSIZE_08_BITS,
        .address_size = QSPI_ADDRSIZE_24_BITS,
        .dummy_cycles = 0,
        .data_size = QSPI_DATASIZE_08_BITS,
        .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
        .data_mode = QSPI_DATA_MODE_SPI,
        .length = 3,
        .clock_stretch_en = 1,
    };

    uint8_t rddata[4];
    uint16_t status = app_qspi_command_receive_sync(APP_QSPI_ID_2, &rd_cmd, rddata, 1000);
    printf("Read Status: %d\n", status);
    printf("RDDID_RESP: 0x%02x 0x%02x 0x%02x 0x%02x\n", rddata[0], rddata[1], rddata[2], rddata[3]);

    rd_cmd.address = 0x000C00;
    rd_cmd.length = 1;
    status = app_qspi_command_receive_sync(APP_QSPI_ID_2, &rd_cmd, rddata, 1000);
    printf("Read Status: %d\n", status);
    printf("RDDCOLMOD_RESP: 0x%02x 0x%02x\n", rddata[0], rddata[1]);

    // Init sequence
    st77916_init_sequence();
}

void display_st77916_psd_360p_set_brightness(uint8_t brightness)
{
    app_pwm_channel_init_t new_config = {
        .duty = brightness,
        .drive_polarity = PWM_DRIVEPOLARITY_POSITIVE,
        .fstoplvl = PWM_STOP_LVL_LOW,
    };
    app_pwm_config_channel(s_pwm_param.id, s_pwm_param.active_channel, &new_config);
    app_pwm_start(s_pwm_param.id);
    printf("Change Brightness to %d\n", brightness);
}

void display_st77916_psd_360p_set_show_area(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2)
{
    static uint16_t last_x1 = 0;
    static uint16_t last_y1 = 0;
    static uint16_t last_x2 = 0;
    static uint16_t last_y2 = 0;

    if (last_x1 == x1 && last_y1 == y1 && last_x2 == x2 && last_y2 == y2)
    {
        return;
    }

    last_x1 = x1;
    last_y1 = y1;
    last_x2 = x2;
    last_y2 = y2;

    uint8_t data[4];

    data[0] = x1 >> 8;
    data[1] = x1 & 0x00ff;
    data[2] = x2 >> 8;
    data[3] = x2 & 0x00ff;
    display_qspi_spi_send_sync(0x2a, data, 4);

    data[0] = (y1 & 0xff00) >> 8;
    data[1] = y1 & 0x00ff;
    data[2] = (y2 & 0xff00) >> 8;
    data[3] = y2 & 0x00ff;
    display_qspi_spi_send_sync(0x2b, data, 4);
}

void display_st77916_psd_360p_wait_te_signal(void)
{
    // return;
#if GR5625_SK
    ll_gpio_enable_it(GPIO0, LL_GPIO_PIN_12);
#else
    ll_aon_gpio_enable_it(LL_AON_GPIO_PIN_5);
#endif // GR5625_SK

    xSemaphoreTake(s_te_sem, 100);

#if GR5625_SK
    ll_gpio_disable_it(GPIO0, LL_GPIO_PIN_12);
#else
    ll_aon_gpio_disable_it(LL_AON_GPIO_PIN_5);
#endif // GR5625_SK
}

static void display_te_evt_callback(app_io_evt_t *p_evt)
{
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR(s_te_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void display_qspi_spi_send_sync(uint32_t cmd, uint8_t *p_data, uint16_t len)
{
    if ((cmd & 0xFF) == cmd)
    {
        cmd <<= 8;
    }

    uint8_t placeholder = 0;
    if (!p_data)
    {
        p_data = &placeholder;
    }

    app_qspi_command_t qspi_cmd = {
        .instruction = 0x02,
        .address = cmd,
        .instruction_size = QSPI_INSTSIZE_08_BITS,
        .address_size = QSPI_ADDRSIZE_24_BITS,
        .dummy_cycles = 0,
        .data_size = QSPI_DATASIZE_08_BITS,
        .instruction_address_mode = QSPI_INST_ADDR_ALL_IN_SPI,
        .length = len,
        .clock_stretch_en = LL_QSPI_CLK_STRETCH_ENABLE,
    };
    portDISABLE_INTERRUPTS();
    app_qspi_command_transmit_sync(s_qspi_param.id, &qspi_cmd, p_data, 0xFFFFFFFFU);
    portENABLE_INTERRUPTS();
}

static void st77916_init_sequence()
{
    uint8_t data[32];
    uint16_t datalen = 0;

    datalen = 0;
    data[datalen++] = 0x08;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x08;
    display_qspi_spi_send_sync(0xF2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x51;
    display_qspi_spi_send_sync(0x9B, data, datalen);

    datalen = 0;
    data[datalen++] = 0x53;
    display_qspi_spi_send_sync(0x86, data, datalen);

    datalen = 0;
    data[datalen++] = 0x80;
    display_qspi_spi_send_sync(0xF2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0xF1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x55;
    display_qspi_spi_send_sync(0xB0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x1E;
    display_qspi_spi_send_sync(0xB1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x3B;
    display_qspi_spi_send_sync(0xB2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x06;
    display_qspi_spi_send_sync(0xB4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x24;
    display_qspi_spi_send_sync(0xB5, data, datalen);

    datalen = 0;
    data[datalen++] = 0xA5;
    display_qspi_spi_send_sync(0xB6, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xB7, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xBA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x08;
    display_qspi_spi_send_sync(0xBB, data, datalen);

    datalen = 0;
    data[datalen++] = 0x08;
    display_qspi_spi_send_sync(0xBC, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xBD, data, datalen);

    datalen = 0;
    data[datalen++] = 0x80;
    display_qspi_spi_send_sync(0xC0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xC1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x37;
    display_qspi_spi_send_sync(0xC2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x80;
    display_qspi_spi_send_sync(0xC3, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xC4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x37;
    display_qspi_spi_send_sync(0xC5, data, datalen);

    datalen = 0;
    data[datalen++] = 0xA9;
    display_qspi_spi_send_sync(0xC6, data, datalen);

    datalen = 0;
    data[datalen++] = 0x41;
    display_qspi_spi_send_sync(0xC7, data, datalen);

    datalen = 0;
    data[datalen++] = 0x51;
    display_qspi_spi_send_sync(0xC8, data, datalen);

    datalen = 0;
    data[datalen++] = 0xA9;
    display_qspi_spi_send_sync(0xC9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x41;
    display_qspi_spi_send_sync(0xCA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x51;
    display_qspi_spi_send_sync(0xCB, data, datalen);

    datalen = 0;
    data[datalen++] = 0x91;
    display_qspi_spi_send_sync(0xD0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x68;
    display_qspi_spi_send_sync(0xD1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x69;
    display_qspi_spi_send_sync(0xD2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0xA5;
    display_qspi_spi_send_sync(0xF5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x3B;
    display_qspi_spi_send_sync(0xDD, data, datalen);

    datalen = 0;
    data[datalen++] = 0x3B;
    display_qspi_spi_send_sync(0xDE, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xF1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0xf0;
    data[datalen++] = 0x0B;
    data[datalen++] = 0x12;
    data[datalen++] = 0x0B;
    data[datalen++] = 0x0A;
    data[datalen++] = 0x06;
    data[datalen++] = 0x39;
    data[datalen++] = 0x43;
    data[datalen++] = 0x4F;
    data[datalen++] = 0x07;
    data[datalen++] = 0x14;
    data[datalen++] = 0x14;
    data[datalen++] = 0x2f;
    data[datalen++] = 0x34;
    display_qspi_spi_send_sync(0xe0, data, datalen);

    datalen = 0;
    data[datalen++] = 0xf0;
    data[datalen++] = 0x0B;
    data[datalen++] = 0x11;
    data[datalen++] = 0x0A;
    data[datalen++] = 0x09;
    data[datalen++] = 0x05;
    data[datalen++] = 0x32;
    data[datalen++] = 0x33;
    data[datalen++] = 0x48;
    data[datalen++] = 0x07;
    data[datalen++] = 0x13;
    data[datalen++] = 0x13;
    data[datalen++] = 0x2C;
    data[datalen++] = 0x33;
    display_qspi_spi_send_sync(0xe1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xF3, data, datalen);

    datalen = 0;
    data[datalen++] = 0x0A;
    display_qspi_spi_send_sync(0xE0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xE1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xE2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xE3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE0;
    display_qspi_spi_send_sync(0xE4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x06;
    display_qspi_spi_send_sync(0xE5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x21;
    display_qspi_spi_send_sync(0xE6, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xE7, data, datalen);

    datalen = 0;
    data[datalen++] = 0x05;
    display_qspi_spi_send_sync(0xE8, data, datalen);

    datalen = 0;
    data[datalen++] = 0xF2;
    display_qspi_spi_send_sync(0xE9, data, datalen);

    datalen = 0;
    data[datalen++] = 0xDF;
    display_qspi_spi_send_sync(0xEA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x80;
    display_qspi_spi_send_sync(0xEB, data, datalen);

    datalen = 0;
    data[datalen++] = 0x20;
    display_qspi_spi_send_sync(0xEC, data, datalen);

    datalen = 0;
    data[datalen++] = 0x14;
    display_qspi_spi_send_sync(0xED, data, datalen);

    datalen = 0;
    data[datalen++] = 0xFF;
    display_qspi_spi_send_sync(0xEE, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xEF, data, datalen);

    datalen = 0;
    data[datalen++] = 0xFF;
    display_qspi_spi_send_sync(0xF8, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xF9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xFA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x30;
    display_qspi_spi_send_sync(0xFB, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xFC, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xFD, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xFE, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xFF, data, datalen);

    datalen = 0;
    data[datalen++] = 0x42;
    display_qspi_spi_send_sync(0x60, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE0;
    display_qspi_spi_send_sync(0x61, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x62, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x63, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x64, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x65, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x66, data, datalen);

    datalen = 0;
    data[datalen++] = 0x03;
    display_qspi_spi_send_sync(0x67, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x68, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x69, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x6A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x6B, data, datalen);

    datalen = 0;
    data[datalen++] = 0x42;
    display_qspi_spi_send_sync(0x70, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE0;
    display_qspi_spi_send_sync(0x71, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x72, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x73, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x74, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x75, data, datalen);

    datalen = 0;
    data[datalen++] = 0x40;
    display_qspi_spi_send_sync(0x76, data, datalen);

    datalen = 0;
    data[datalen++] = 0x03;
    display_qspi_spi_send_sync(0x77, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x78, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x79, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x7A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x7B, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0x80, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x81, data, datalen);

    datalen = 0;
    data[datalen++] = 0x05;
    display_qspi_spi_send_sync(0x82, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x83, data, datalen);

    datalen = 0;
    data[datalen++] = 0xDD;
    display_qspi_spi_send_sync(0x84, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x85, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x86, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x87, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0x88, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x89, data, datalen);

    datalen = 0;
    data[datalen++] = 0x07;
    display_qspi_spi_send_sync(0x8A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x8B, data, datalen);

    datalen = 0;
    data[datalen++] = 0xDF;
    display_qspi_spi_send_sync(0x8C, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x8D, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x8E, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x8F, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0x90, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x91, data, datalen);

    datalen = 0;
    data[datalen++] = 0x09;
    display_qspi_spi_send_sync(0x92, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x93, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE1;
    display_qspi_spi_send_sync(0x94, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x95, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x96, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x97, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0x98, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x99, data, datalen);

    datalen = 0;
    data[datalen++] = 0x0B;
    display_qspi_spi_send_sync(0x9A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0x9B, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE3;
    display_qspi_spi_send_sync(0x9C, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x9D, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x9E, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x9F, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0xA0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xA1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x04;
    display_qspi_spi_send_sync(0xA2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xDC;
    display_qspi_spi_send_sync(0xA4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xA5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xA6, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xA7, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0xA8, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xA9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x06;
    display_qspi_spi_send_sync(0xAA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0xAB, data, datalen);

    datalen = 0;
    data[datalen++] = 0xDE;
    display_qspi_spi_send_sync(0xAC, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xAD, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xAE, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xAF, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0xB0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xB1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x08;
    display_qspi_spi_send_sync(0xB2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0xB3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE0;
    display_qspi_spi_send_sync(0xB4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xB5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xB6, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xB7, data, datalen);

    datalen = 0;
    data[datalen++] = 0x48;
    display_qspi_spi_send_sync(0xB8, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xB9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x0A;
    display_qspi_spi_send_sync(0xBA, data, datalen);

    datalen = 0;
    data[datalen++] = 0x02;
    display_qspi_spi_send_sync(0xBB, data, datalen);

    datalen = 0;
    data[datalen++] = 0xE2;
    display_qspi_spi_send_sync(0xBC, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xBD, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xBE, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xBF, data, datalen);

    datalen = 0;
    data[datalen++] = 0x12;
    display_qspi_spi_send_sync(0xC0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x88;
    display_qspi_spi_send_sync(0xC1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x65;
    display_qspi_spi_send_sync(0xC2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x74;
    display_qspi_spi_send_sync(0xC3, data, datalen);

    datalen = 0;
    data[datalen++] = 0x47;
    display_qspi_spi_send_sync(0xC4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x56;
    display_qspi_spi_send_sync(0xC5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xC6, data, datalen);

    datalen = 0;
    data[datalen++] = 0xAA;
    display_qspi_spi_send_sync(0xC7, data, datalen);

    datalen = 0;
    data[datalen++] = 0xBB;
    display_qspi_spi_send_sync(0xC8, data, datalen);

    datalen = 0;
    data[datalen++] = 0x33;
    display_qspi_spi_send_sync(0xC9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x21;
    display_qspi_spi_send_sync(0xD0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x88;
    display_qspi_spi_send_sync(0xD1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x65;
    display_qspi_spi_send_sync(0xD2, data, datalen);

    datalen = 0;
    data[datalen++] = 0x74;
    display_qspi_spi_send_sync(0xD3, data, datalen);

    datalen = 0;
    data[datalen++] = 0x47;
    display_qspi_spi_send_sync(0xD4, data, datalen);

    datalen = 0;
    data[datalen++] = 0x56;
    display_qspi_spi_send_sync(0xD5, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xD6, data, datalen);

    datalen = 0;
    data[datalen++] = 0xAA;
    display_qspi_spi_send_sync(0xD7, data, datalen);

    datalen = 0;
    data[datalen++] = 0xBB;
    display_qspi_spi_send_sync(0xD8, data, datalen);

    datalen = 0;
    data[datalen++] = 0x33;
    display_qspi_spi_send_sync(0xD9, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0xF3, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0xF1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x0B;
    display_qspi_spi_send_sync(0xA0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x2A;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x2B;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x2C;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x2D;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x2E;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x2F;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x30;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x31;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x32;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x33;
    display_qspi_spi_send_sync(0xA3, data, datalen);

    datalen = 0;
    data[datalen++] = 0xC3;
    display_qspi_spi_send_sync(0xA5, data, datalen);
    delay_ms(1);

    datalen = 0;
    data[datalen++] = 0x09;
    display_qspi_spi_send_sync(0xA0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x10;
    display_qspi_spi_send_sync(0xF1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0xF0, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x35, data, datalen);

    display_qspi_spi_send_sync(0xF4, NULL, 0);

    datalen = 0;
    data[datalen++] = 0x20;
    display_qspi_spi_send_sync(0xC1, data, datalen);

    datalen = 0;
    data[datalen++] = 0x60;
    display_qspi_spi_send_sync(0xC2, data, datalen);

    display_qspi_spi_send_sync(0xF4, NULL, 0);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x44, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x36, data, datalen);

    datalen = 0;
    data[datalen++] = 0x55;
    display_qspi_spi_send_sync(0x3A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0x00;
    data[datalen++] = 0x01;
    data[datalen++] = 0x67;
    display_qspi_spi_send_sync(0x2A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0x00;
    data[datalen++] = 0x01;
    data[datalen++] = 0x67;
    display_qspi_spi_send_sync(0x2B, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x4D, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x4E, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x4F, data, datalen);

    datalen = 0;
    data[datalen++] = 0x01;
    display_qspi_spi_send_sync(0x4C, data, datalen);
    delay_ms(10);

    datalen = 0;
    data[datalen++] = 0x00;
    display_qspi_spi_send_sync(0x4C, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0x00;
    data[datalen++] = 0x01;
    data[datalen++] = 0x67;
    display_qspi_spi_send_sync(0x2A, data, datalen);

    datalen = 0;
    data[datalen++] = 0x00;
    data[datalen++] = 0x00;
    data[datalen++] = 0x01;
    data[datalen++] = 0x67;
    display_qspi_spi_send_sync(0x2B, data, datalen);

    display_qspi_spi_send_sync(0x21, NULL, 0);
    display_qspi_spi_send_sync(0x11, NULL, 0);
    delay_ms(120);

    display_qspi_spi_send_sync(0x29, NULL, 0);
}
