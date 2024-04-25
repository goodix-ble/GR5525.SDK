#include "custom_config.h"
#include "ble.h"
#include "ble_event.h"
#include "patch_tab.h"

extern void adv_func_init(void);
extern void scan_func_init(void);
extern void master_func_init(void);
extern void slave_func_init(void);
extern void legacy_pair_func_init(void);
extern void sc_pair_func_init(void);
extern void coc_func_init(void);
extern void gatts_func_init(void);
extern void gattc_func_init(void);
extern void conn_aoa_aod_func_init(void);
extern void connless_aoa_aod_func_init(void);
extern void ranging_func_init(void);
extern void power_control_func_init(void);

extern void ble_common_env_init(void);
extern void ble_con_env_init(void);
extern void ble_scan_env_init(void);
extern void ble_adv_env_init(void);
extern void ble_test_evn_init(void);
extern void ble_iso_env_init(void);
extern void ble_car_key_env_init(void);
extern void ble_bt_bredr_env_init(void);
extern void ble_mul_link_env_init(void);

extern void reg_ke_msg_patch_tab(ke_msg_tab_item_t *ke_msg_tab, uint16_t ke_msg_cnt);
extern void reg_gapm_hci_evt_patch_tab(gapm_hci_evt_tab_item_t *gapm_hci_evt_tab, uint16_t gapm_hci_evt_cnt);
extern void reg_llm_hci_cmd_patch_tab(llm_hci_cmd_tab_item_t *hci_cmd_tab, uint16_t hci_cmd_cnt);

extern void ble_stack_enable(ble_evt_handler_t evt_handler, stack_heaps_table_t *p_heaps_table);
extern void ble_task_tab_init(void);

void ble_feature_init(void)
{
    #if (CFG_MAX_ADVS)
    adv_func_init();
    #endif

    #if (CFG_MAX_SCAN)
    scan_func_init();
    #endif

    #if (CFG_MASTER_SUPPORT)
    master_func_init();
    #endif

    #if (CFG_SLAVE_SUPPORT)
    slave_func_init();
    #endif

    #if (CFG_LEGACY_PAIR_SUPPORT)
    legacy_pair_func_init();
    #endif

    #if (CFG_SC_PAIR_SUPPORT)
    sc_pair_func_init();
    #endif

    #if (CFG_COC_SUPPORT)
    coc_func_init();
    #endif

    #if (CFG_GATTS_SUPPORT)
    gatts_func_init();
    #endif

    #if (CFG_GATTC_SUPPORT)
    gattc_func_init();
    #endif

    #if (CFG_CONN_AOA_AOD_SUPPORT)
    conn_aoa_aod_func_init();
    #endif

    #if (CFG_CONNLESS_AOA_AOD_SUPPORT)
    connless_aoa_aod_func_init();
    #endif

    #if (CFG_RANGING_SUPPORT)
    ranging_func_init();
    #endif

    #if (CFG_POWER_CONTROL_SUPPORT)
    power_control_func_init();
    #endif
}

void ble_sdk_patch_env_init(void)
{
    // register the msg handler for patch
    uint16_t ke_msg_cnt = sizeof(ke_msg_tab) / sizeof(ke_msg_tab_item_t);
    reg_ke_msg_patch_tab(ke_msg_tab, ke_msg_cnt);

    // register the llm hci cmd handler for patch
    uint16_t llm_hci_cmd_cnt = sizeof(llm_hci_cmd_tab) / sizeof(llm_hci_cmd_tab_item_t);
    reg_llm_hci_cmd_patch_tab(llm_hci_cmd_tab, llm_hci_cmd_cnt);

    // register the gapm hci evt handler for patch
    uint16_t gapm_hci_evt_cnt = sizeof(gapm_hci_evt_tab) / sizeof(gapm_hci_evt_tab_item_t);
    reg_gapm_hci_evt_patch_tab(gapm_hci_evt_tab, gapm_hci_evt_cnt);

    ble_common_env_init();

    #if CFG_MAX_CONNECTIONS
    ble_con_env_init();
    #endif

    #if CFG_MAX_SCAN
    ble_scan_env_init();
    #endif

    #if CFG_MAX_ADVS
    ble_adv_env_init();
    #endif

    #if DTM_TEST_ENABLE
    ble_test_evn_init();
    #endif

    #if CFG_CAR_KEY_SUPPORT
    ble_car_key_env_init();
    #endif

    #if CFG_BT_BREDR
    ble_bt_bredr_env_init();
    #endif

    #if CFG_MUL_LINK_WITH_SAME_DEV
    ble_mul_link_env_init();
    #endif
}

void ble_stack_init(ble_evt_handler_t evt_handler, stack_heaps_table_t *p_heaps_table)
{
    ble_sdk_patch_env_init();
    ble_feature_init();
    ble_task_tab_init();
    ble_stack_enable(evt_handler, p_heaps_table);
}

