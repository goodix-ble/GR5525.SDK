#include "string.h"
#include "qspi_flash.h"
#include "qspi_psram.h"
#include "qspi_screen_390.h"
#include "app_graphics_qspi.h"

#include "image/hehua_454_rgba565.h"
#include "image/tiger_454_rgba565.h"


/*
 * Display Device Setting
 */
#define Q_DISPLAY_QSPI_ID                           APP_QSPI_ID_2
#define Q_DISPLAY_CLOCK_PREESCALER                  2u
#define Q_DISPLAY_PIXEL_WIDTH                       (454u)              /* screen width */
#define Q_DISPLAY_PIXEL_HEIGHT                      (454u)              /* screen height */
#define Q_DISPLAY_PIXEL_DEPTH                       (2u)

/*
 * Flash Device Setting
 */
#define Q_NOR_FLASH_QSPI_ID                         APP_QSPI_ID_0
#define Q_NOR_FLASH_CLOCK_PREESCALER                2u
#define Q_NOR_FLASH_PIN_GROUP                       QSPI0_PIN_GROUP_0
#define Q_NOR_FLASH_SECTOR_SIZE                     4096
#define Q_NOR_FLASH_PAGE_SIZE                       256
#define Q_NOR_FLASH_TEST_ADDRESS                    0x00003000
#define Q_NOR_FLASH_TEST_SIZE                       (Q_DISPLAY_PIXEL_WIDTH*Q_DISPLAY_PIXEL_HEIGHT*Q_DISPLAY_PIXEL_DEPTH)


/*
 * PSRAM Device Setting
 */
#define Q_PSRAM_QSPI_ID                             APP_QSPI_ID_1
#define Q_PSRAM_CLOCK_PREESCALER                    2u
#define Q_PSRAM_PIN_GROUP                           QSPI1_PIN_GROUP_0
#define Q_PSRAM_TEST_ADDRESS                        0x00000000
#define Q_PSRAM_SECTOR_SIZE                         4096
#define Q_PSRAM_PAGE_SIZE                           256
#define Q_PSRAM_TEST_SIZE                           (Q_DISPLAY_PIXEL_WIDTH*Q_DISPLAY_PIXEL_HEIGHT*Q_DISPLAY_PIXEL_DEPTH)

static volatile bool is_display_cmplt = false;

static uint8_t  qspi_dev_init_flash(void);
static uint8_t  qspi_dev_init_psram(void);
static uint8_t  qspi_dev_init_display(void);
static void     qspi_display_clear_flag(void);
static void     qspi_display_wait_cplt(void);

/**********************************************************************************
 *                        PUBLIC   METHODs
 **********************************************************************************/

void qspi_devs_init(void) {
    qspi_dev_init_flash();
    qspi_dev_init_psram();
    qspi_dev_init_display();
}

void qspi_display_refresh(void) {

    app_qspi_screen_command_t   screen_cmd;
    app_qspi_screen_info_t      screen_info;
    app_qspi_screen_scroll_t    scroll_config;

    screen_cmd.instruction              = 0x12;
    screen_cmd.instruction_size         = QSPI_INSTSIZE_08_BITS;
    screen_cmd.leading_address          = 0x002C00;
    screen_cmd.ongoing_address          = 0x003C00;
    screen_cmd.address_size             = QSPI_ADDRSIZE_24_BITS;
    screen_cmd.data_size                = QSPI_DATASIZE_16_BITS;
    screen_cmd.instruction_address_mode = QSPI_INST_IN_SPI_ADDR_IN_SPIFRF;
    screen_cmd.dummy_cycles             = 0;
    screen_cmd.data_mode                = QSPI_DATA_MODE_QUADSPI;

    screen_info.scrn_pixel_width  = Q_DISPLAY_PIXEL_WIDTH;
    screen_info.scrn_pixel_height = Q_DISPLAY_PIXEL_HEIGHT;
    screen_info.scrn_pixel_depth  = Q_DISPLAY_PIXEL_DEPTH;
#if 1
    scroll_config.first_frame_start_address  = app_qspi_get_xip_base_address(Q_PSRAM_QSPI_ID) + Q_PSRAM_TEST_ADDRESS;          /* Load from external qspi-psram */
#else
    scroll_config.first_frame_start_address  = app_qspi_get_xip_base_address(Q_NOR_FLASH_QSPI_ID) + Q_NOR_FLASH_TEST_ADDRESS;      /* Load from external qspi-flash */
#endif
    scroll_config.second_frame_start_address = (uint32_t)&hehua_454_rgba565[0];                             /* Load from xqspi-flash */
    scroll_config.is_horizontal_scroll  = false;//true;

    static uint32_t scroll_point = 0;
    qspi_screen_set_show_area(0, Q_DISPLAY_PIXEL_WIDTH - 1, 0, Q_DISPLAY_PIXEL_HEIGHT - 1);
    scroll_config.scroll_coordinate = scroll_point;

    qspi_display_clear_flag();
#if 1
    app_qspi_async_draw_screen(Q_DISPLAY_QSPI_ID, Q_PSRAM_QSPI_ID, &screen_cmd, &screen_info, &scroll_config, true);
#else
    app_qspi_async_draw_screen(Q_DISPLAY_QSPI_ID, Q_NOR_FLASH_QSPI_ID, &screen_cmd, &screen_info, &scroll_config, true);
#endif
    qspi_display_wait_cplt();

    scroll_point++;
    scroll_point = scroll_point % Q_DISPLAY_PIXEL_WIDTH;

    return;
}

void qspi_dev_application_task(void *p_arg) {
    qspi_devs_init();

    while(1) {
        qspi_display_refresh();
        printf(".");
    }
}


/**********************************************************************************
 *                        STATIC   METHODs
 **********************************************************************************/

static uint8_t qspi_dev_init_flash(void) {
    uint8_t flash_id = 0;
    uint32_t i = 0, counts;

    flash_id = SPI_FLASH_init(Q_NOR_FLASH_QSPI_ID, Q_NOR_FLASH_CLOCK_PREESCALER, Q_NOR_FLASH_PIN_GROUP);
    if(flash_id != 0x0b) {
        printf(">>> Q.Flash Init Failed!\r\n");
        goto mmap_set;
    }

    printf("Program Flash : \r\n");
    counts = ((413*1024)/Q_NOR_FLASH_PAGE_SIZE);
    SPI_FLASH_Chip_Erase();
    SPI_FLASH_Enable_Quad();
    delay_ms(1);
    for(i = 0; i < counts; i++) {
        SPI_FLASH_Quad_Page_Program_With_Data_Size(Q_NOR_FLASH_TEST_ADDRESS + i * Q_NOR_FLASH_PAGE_SIZE, (uint8_t *)&hehua_454_rgba565[i * Q_NOR_FLASH_PAGE_SIZE],QSPI_FLASH_DATA_SIZE_08_BITS);
        if(i % 16 == 0)
            printf("%d%% ", i*100/counts);
    }
    printf("\r\n\r\n");

mmap_set:
    delay_ms(1);

    app_qspi_mmap_device_t dev = {
        .dev_type    = APP_QSPI_DEVICE_FLASH,
        .rd.flash_rd = FLASH_MMAP_CMD_DREAD_3BH,
    };
    app_qspi_config_memory_mappped(Q_NOR_FLASH_QSPI_ID, dev);

    printf(">>> Q.Flash Init OK!\r\n");

    return flash_id;
}


static uint8_t qspi_dev_init_psram(void) {
    uint8_t psram_id = 0;

    psram_id = qspi_psram_init(Q_PSRAM_QSPI_ID, Q_PSRAM_CLOCK_PREESCALER, Q_PSRAM_PIN_GROUP, QSPI_CLOCK_MODE_3);
    if(psram_id != 0x5D) {
        printf(">>> Q.PSRAM Init Failed!\r\n");
        return 0x00;
    }
    else {
        printf(">>> Q.PSRAM Init OK!\r\n");
    }

    app_qspi_mmap_device_t dev = {
        .dev_type    = APP_QSPI_DEVICE_PSRAM,
        .rd.psram_rd = PSRAM_MMAP_CMD_QREAD_EBH,
        .psram_wr    = PSRAM_MMAP_CMD_QWRITE_02H,
    };

    app_qspi_config_memory_mappped(Q_PSRAM_QSPI_ID,  dev);

    memcpy((void*) (app_qspi_get_xip_base_address(Q_PSRAM_QSPI_ID) + Q_PSRAM_TEST_ADDRESS), (void*)&tiger_454_rgba565[0], sizeof(tiger_454_rgba565));

    if(memcmp((void*)(app_qspi_get_xip_base_address(Q_PSRAM_QSPI_ID) + Q_PSRAM_TEST_ADDRESS), (void*)&tiger_454_rgba565[0], sizeof(tiger_454_rgba565)) != 0) {
        printf(">>>*****Q.PSRAM ERROR DATA !!!*****\r\n");
    } else {
        printf(">>>Q.PSRAM Right DATA !!!\r\n");
    }

    return psram_id;
}

static void display_qspi_evt_handler(app_qspi_evt_t *p_evt) {
    switch(p_evt->type) {
        case APP_QSPI_EVT_ASYNC_WR_SCRN_CPLT:
            is_display_cmplt = true;
            break;

        case APP_QSPI_EVT_ASYNC_WR_SCRN_FAIL:
            is_display_cmplt = true;
            break;

        default:
            break;
    }
}

static uint8_t qspi_dev_init_display(void) {
    rm69330_lcd_res_e default_res = RM69330_LCD_RES_454;
    qspi_screen_init_basic(Q_DISPLAY_QSPI_ID, Q_DISPLAY_CLOCK_PREESCALER, default_res, display_qspi_evt_handler);
    printf(">>> Q.Display Init OK!\r\n");
    return 0;
}

static void qspi_display_clear_flag(void) {
    is_display_cmplt = false;
}

static void qspi_display_wait_cplt(void) {
    while(!is_display_cmplt){}
}
