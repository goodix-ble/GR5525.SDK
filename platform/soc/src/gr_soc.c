#include "gr_soc.h"

#ifndef DRIVER_TEST
#include "gr_includes.h"
#endif

#include "hal_flash.h"
#include "platform_sdk.h"
#include "pmu_calibration.h"
#include "app_pwr_mgmt.h"

#define PUYA_FLASH_HP_CMD              (0xA3)
#define PUYA_FLASH_HP_END_DUMMY        (2)
#define FALSH_HP_MODE                  XQSPI_HP_MODE_DIS
#define FLASH_HP_CMD                   PUYA_FLASH_HP_CMD
#define FLASH_HP_END_DUMMY             PUYA_FLASH_HP_END_DUMMY
#define SOFTWARE_REG_WAKEUP_FLAG_POS   (8)

/******************************************************************************/

#define SDK_VER_MAJOR                   1
#define SDK_VER_MINOR                   0
#define SDK_VER_BUILD                   1
#define COMMIT_ID                       0xecd527bd

static const sdk_version_t sdk_version = {SDK_VER_MAJOR,
                                          SDK_VER_MINOR,
                                          SDK_VER_BUILD,
                                          COMMIT_ID,};//sdk version

void sys_sdk_verison_get(sdk_version_t *p_version)
{
    memcpy(p_version, &sdk_version, sizeof(sdk_version_t));
}

uint32_t SystemCoreClock = CLK_64M;  /* System Core Clock Frequency as 64Mhz     */

__ALIGNED(0x400) FuncVector_t FuncVector_table[MAX_NUMS_IRQn + NVIC_USER_IRQ_OFFSET] = {
    0,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
};

void soc_register_nvic(IRQn_Type indx, uint32_t func)
{
    FuncVector_table[indx + 16] = (FuncVector_t)func;
}

__WEAK void nvds_init_error_handler(uint8_t err_code)
{
    /* nvds_deinit will erase the flash area and old data will be lost */
#ifdef NVDS_START_ADDR
    nvds_deinit(NVDS_START_ADDR, NVDS_NUM_SECTOR);
    nvds_init(NVDS_START_ADDR, NVDS_NUM_SECTOR);
#else
    nvds_deinit(0, NVDS_NUM_SECTOR);
    nvds_init(0, NVDS_NUM_SECTOR);
#endif
}

static void nvds_setup(void)
{
#ifndef ATE_TEST_ENABLE
    nvds_retention_size(4);

#ifdef NVDS_START_ADDR
    uint8_t err_code = nvds_init(NVDS_START_ADDR, NVDS_NUM_SECTOR);
#else
    uint8_t err_code = nvds_init(0, NVDS_NUM_SECTOR);
#endif

    switch(err_code)
    {
        case NVDS_SUCCESS:
            break;
        default:
            /* Nvds initialization errors.
             * For more information, please see NVDS_INIT_ERR_CODE. */
            nvds_init_error_handler(err_code);
            break;
    }
#endif //ATE_TEST_ENABLE
}

void sys_device_reset_reason_enable(void)
{
    //Clear all status and enable reset record
    AON_CTL->DBG_REG_RST_SRC = (1UL << 24) | (1UL << 31);
    //Busy configuration, Wait
    while (AON_CTL->DBG_REG_RST_SRC & (1UL << 30));
    //Wait record ready
    while (!(AON_CTL->DBG_REG_RST_SRC & (1UL << 16)));
}

uint8_t sys_device_reset_reason(void)
{
    uint8_t reset_season = AON_CTL->DBG_REG_RST_SRC & 0x1FUL;

    //Clear all status
    AON_CTL->DBG_REG_RST_SRC = ((1UL << 25) | (1UL << 31));
    //Busy configuration, Wait
    while (AON_CTL->DBG_REG_RST_SRC & (1UL << 30));
    //enable reset record
    AON_CTL->DBG_REG_RST_SRC = (1UL << 24) | (1UL << 31);
    //Busy configuration, Wait
    while (AON_CTL->DBG_REG_RST_SRC & (1UL << 30));
    //Wait record ready
    while (!(AON_CTL->DBG_REG_RST_SRC & (1UL << 16)));
    if(reset_season & SYS_RESET_REASON_AONWDT)
    {
        return SYS_RESET_REASON_AONWDT;
    }
    else
    {
        return SYS_RESET_REASON_NONE;
    }
}

void first_class_task(void)
{
    exflash_hp_init_t hp_init;

    platform_xqspi_env_init();
    if (!hal_flash_init())
    {
        /* Flash fault, cannot startup.
         * TODO: Output log via UART or Dump an error code to flash. */
        while(1);
    }

    hp_init.xqspi_hp_enable    = FALSH_HP_MODE;
    hp_init.xqspi_hp_cmd       = FLASH_HP_CMD;
    hp_init.xqspi_hp_end_dummy = FLASH_HP_END_DUMMY;
    platform_flash_enable_quad(&hp_init);

    /* Enable chip reset reason record. */
    sys_device_reset_reason_enable();
    /* nvds module init process. */
    nvds_setup();

    /* IO leakage protect configuration. */
    system_io_leakage_protect();

    /* platform init process. */
    platform_sdk_init();
}

void second_class_task(void)
{
#if CFG_LPCLK_INTERNAL_EN
    platform_clock_init((mcu_clock_type_t)SYSTEM_CLOCK, (sdk_clock_type_t)RNG_OSC_CLK2, CFG_LF_ACCURACY_PPM, 0);
#else
    platform_clock_init((mcu_clock_type_t)SYSTEM_CLOCK, RTC_OSC_CLK, CFG_LF_ACCURACY_PPM, 0);
#endif

#if PMU_CALIBRATION_ENABLE && !defined(DRIVER_TEST)
    /* Enable auto pmu calibration function. */
    if(!CHECK_IS_ON_FPGA())
    {
        system_pmu_calibration_init(30000);
    }
#endif

    system_pmu_init((mcu_clock_type_t)SYSTEM_CLOCK);
    SystemCoreSetClock((mcu_clock_type_t)SYSTEM_CLOCK);
    SetSerialClock(SERIAL_N96M_CLK);

    // recover the default setting by temperature, should be called in the end
    if(!CHECK_IS_ON_FPGA())
    {
        pmu_calibration_handler(NULL);
    }

    /* Init peripheral sleep management */
    app_pwr_mgmt_init();
}

static fun_t svc_user_func = NULL;

void svc_func_register(uint8_t svc_num, uint32_t user_func)
{
    svc_user_func = (fun_t)user_func;
}

void svc_user_handler(uint8_t svc_num)
{
    if (svc_user_func)
        svc_user_func();
}


static void patch_init(void)
{
    gr5xx_fpb_init(FPB_MODE_PATCH_AND_DEBUG);
}

void platform_init(void)
{
    patch_init();
    first_class_task();
    second_class_task();
}

boot_mode_t pwr_mgmt_get_wakeup_flag(void)
{
    if ( AON_CTL->SOFTWARE_REG0 & (1 << SOFTWARE_REG_WAKEUP_FLAG_POS) )
    {
        return WARM_BOOT;
    }
    return COLD_BOOT;
}

void vector_table_init(void)
{
    __DMB(); // Data Memory Barrier
    FuncVector_table[0] = *(FuncVector_t *)(SCB->VTOR);
    SCB->VTOR = (uint32_t)FuncVector_table; // Set VTOR to the new vector table location
    __DSB(); // Data Synchronization Barrier to ensure all
}

void warm_boot_process(void)
{
    vector_table_init();
    pwr_mgmt_warm_boot();
}

void soc_init(void)
{
    platform_init();
}

__WEAK void sdk_init(void){};
