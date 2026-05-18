#ifndef __LV_LAYOUT_ROUTER_H__
#define __LV_LAYOUT_ROUTER_H__

#include "lvgl.h"

enum {
    TILEVIEW_MAP_ID_MAIN_SCREEN = 0,
    TILEVIEW_MAP_ID_RESERVED_01,
    TILEVIEW_MAP_ID_SETTINGS,
    TILEVIEW_MAP_ID_APP_LIST,
    TILEVIEW_MAP_ID_HR_CHART,
    TILEVIEW_MAP_ID_OTA,
    TILEVIEW_MAP_ID_SWITCH_WF,
    // TILEVIEW_MAP_ID_BT_SCAN_DEV,
    TILEVIEW_MAP_ID_MAX,
};

/** @brief Return to previous page.
*/
void lv_layout_router_back(lv_obj_t * from_obj);

void lv_layout_router_show_settings(lv_obj_t * from_obj);

void lv_layout_router_show_app_list(lv_obj_t * from_obj);

void lv_layout_router_show_heartrate_chart(lv_obj_t * from_obj);

void lv_layout_router_show_ota(lv_obj_t * from_obj);

void lv_layout_router_show_watchface_switch(lv_obj_t *from_obj);

void lv_layout_router_show_calling_layout(lv_obj_t *from_obj);

void lv_layout_router_show_factory_reset(lv_obj_t *from_obj);

#endif // __LV_LAYOUT_ROUTER_H__

