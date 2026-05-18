#include "lvgl.h"
#include "lv_layout_router.h"

extern lv_obj_t * lv_menulist_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t * lv_ecg_control_layout_create(lv_obj_t * parent_tv_obj);

extern lv_obj_t * lv_watchface_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t * lv_watchface_switch_layout_create(lv_obj_t *parent_tv_obj);

extern lv_obj_t *lv_layout_tv_top_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_layout_tv_bottom_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_layout_tv_left_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_layout_tv_right_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_layout_tv_mid_create(lv_obj_t * parent_tv_obj);

extern lv_obj_t *lv_activity_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_status_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_weather_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_notification_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_heartrate_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_ota_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_list_entry_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_heartrate_chart_layout_create(lv_obj_t * parent_tv_obj);
extern lv_obj_t *lv_app_list_layout_create(lv_obj_t * parent_tv_obj);

#if LV_GDX_PATCH_USE_FAST_TILEVIEW
#else
static void lv_layout_tv_load_win(uint8_t row_id, uint8_t col_id, lv_obj_t * tv_obj);
static void lv_layout_tv_scroll_event_handler(lv_event_t * e);
#endif

void lv_layout_tv_init(void);

void lv_layout_startup(void) {
    #if LV_GDX_PATCH_USE_FAST_TILEVIEW
    extern void _init_fast_tileview(void);
    _init_fast_tileview();
    #else
    lv_layout_tv_init();
    #endif
}


#if LV_GDX_PATCH_USE_FAST_TILEVIEW
lv_obj_t * layout_router(lv_obj_t * obj, int map_id, int new_row, int new_col, lv_fast_tileview_transition_effect_t * p_effect)
{
    printf("==> SHOW TILE: ID=%d, ROW=%d, COL=%d\n", map_id, new_row, new_col);
    lv_obj_set_style_bg_color(obj, lv_color_make(0x0, 0x0, 0x0), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    // 弹出的菜单界面地图
    if (map_id == TILEVIEW_MAP_ID_SETTINGS)
    {
        if (new_row == 0) {
            if (new_col == 0) {
                return lv_notification_layout_create(obj);
            }
            if (new_col == 1) {
                // 实现侧滑返回的效果，向左滑动就返回。
                // 此时该函数必须return NULL，否则最终显示的是该函数return的值。
                lv_fast_tileview_pop(obj);
            }
        }
        return NULL;
    }
    if (map_id == TILEVIEW_MAP_ID_APP_LIST)
    {
        if (new_row == 0 && new_col == 0){
            return lv_app_list_layout_create(obj);
        }
        // else {
        //     // 任何方向滑动都退出
        //     lv_fast_tileview_pop(obj);
        // }
        if (new_row == 0 && new_col == 1)
        {
            // 向左滑动就返回。
            lv_fast_tileview_pop(obj);
        }
        return NULL;
    }
    if (map_id == TILEVIEW_MAP_ID_HR_CHART)
    {
        if (new_row == 0 && new_col == 0){
            return lv_heartrate_chart_layout_create(obj);
        }
        if (new_row == 0 && new_col == -1)
        {
            // 向右滑动就返回。
            lv_fast_tileview_pop(obj);
        }
        return NULL;
    }
    if (map_id == TILEVIEW_MAP_ID_OTA)
    {
        if (new_row == 0) {
            if (new_col == 0) {
                return lv_ota_layout_create(obj);
            }
            if (new_col == 1) {
                // 侧滑返回
                lv_fast_tileview_pop(obj);
            }
        }
        return NULL;
    }
    if (map_id == TILEVIEW_MAP_ID_SWITCH_WF)
    {
        if (new_row == 0) {
            if (new_col == 0) {
                return lv_watchface_switch_layout_create(obj);
            }
            if (new_col == 1) {
//                lv_fast_tileview_pop(obj);
            }
        }
        return NULL;
    }

    // 主界面地图
    switch (new_row)
    {
    case -1:
        if (new_col == 0) // 上中
        {
            return lv_status_layout_create(obj);
        }
        break;

    case 0:
        if (new_col == -1) // 中左
        {
            return lv_list_entry_layout_create(obj);
        }
        else if (new_col == 0) // 中中
        {
            return lv_watchface_layout_create(obj);
        }
        else if (new_col == 1) // 中右
        {
            return lv_heartrate_layout_create(obj);
        }
        else if (new_col == 2)
        {
            return lv_watchface_layout_create(obj);
        }
        break;

    case 1:
        if (new_col == 0) // 下中
        {
            return lv_notification_layout_create(obj);
        }
        break;

    default:
        break;
    }
    return NULL;
}

void _init_fast_tileview(void)
{
    lv_obj_t * scr = lv_scr_act();
    lv_fast_tileview_t* tv = lv_fast_tileview_create(scr, layout_router);
    lv_obj_set_size(&tv->obj, LV_PCT(100), LV_PCT(100));
    tv->effect = LV_FAST_TILEVIEW_EFFECT_TRANSLATION;
}

bool lv_is_tileview_scrolling(void) {
    return false;
}

//override the lv_timer_is_filtered
bool lv_timer_is_filtered(lv_timer_t * timer) {
    return false;
}

void lv_layout_router_back(lv_obj_t * from_obj)
{
    lv_fast_tileview_pop(from_obj);
}

void lv_layout_router_show_settings(lv_obj_t * from_obj)
{
    lv_fast_tileview_push(from_obj, TILEVIEW_MAP_ID_SETTINGS, 0, 0);
}

void lv_layout_router_show_app_list(lv_obj_t * from_obj)
{
    lv_fast_tileview_push(from_obj, TILEVIEW_MAP_ID_APP_LIST, 0, 0);
}

void lv_layout_router_show_heartrate_chart(lv_obj_t * from_obj)
{
    lv_fast_tileview_push(from_obj, TILEVIEW_MAP_ID_HR_CHART, 0, 0);
}

void lv_layout_router_show_ota(lv_obj_t * from_obj)
{
    lv_fast_tileview_push(from_obj, TILEVIEW_MAP_ID_OTA, 0, 0);
}

void lv_layout_router_show_watchface_switch(lv_obj_t *from_obj)
{
    lv_fast_tileview_push(from_obj, TILEVIEW_MAP_ID_SWITCH_WF, 0, 0);
}

#else
void lv_layout_tv_init(void) {

    s_tv._tv_root = lv_tileview_create(lv_scr_act());

    lv_obj_set_scrollbar_mode(s_tv._tv_root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(s_tv._tv_root, LV_OBJ_FLAG_SCROLLABLE);

    /**** Laylout
            ------
            |    |
            |    |
       ----------------
       |    |    |    |
       |    |    |    |
       ----------------
            |    |
            |    |
            ------

     ***/

    s_tv._tv_01 = lv_tileview_add_tile2(s_tv._tv_root, 0, 1, LV_DIR_BOTTOM);
    s_tv._tv_10 = lv_tileview_add_tile2(s_tv._tv_root, 1, 0, LV_DIR_RIGHT);
    s_tv._tv_11 = lv_tileview_add_tile2(s_tv._tv_root, 1, 1, LV_DIR_ALL);
    s_tv._tv_12 = lv_tileview_add_tile2(s_tv._tv_root, 1, 2, LV_DIR_LEFT);
    s_tv._tv_21 = lv_tileview_add_tile2(s_tv._tv_root, 2, 1, LV_DIR_TOP);

    lv_layout_tv_load_win(0, 1, s_tv._tv_01);
    lv_layout_tv_load_win(1, 0, s_tv._tv_10);
    lv_layout_tv_load_win(1, 1, s_tv._tv_11);
    lv_layout_tv_load_win(1, 2, s_tv._tv_12);
    lv_layout_tv_load_win(2, 1, s_tv._tv_21);

    lv_obj_set_tile_id(s_tv._tv_root, 2, 1, LV_ANIM_OFF);
    lv_obj_add_event_cb(s_tv._tv_root, lv_layout_tv_scroll_event_handler, LV_EVENT_ALL, NULL);
}


static void lv_layout_tv_load_win(uint8_t row_id, uint8_t col_id, lv_obj_t * tv_obj) {

    switch(_TV_RowCol_ID(row_id, col_id)) {
        case _TV_RowCol_ID(0, 1):
        {
            lv_status_layout_create(tv_obj);
        }
        break;

        case _TV_RowCol_ID(1, 0):
        {
            lv_list_entry_layout_create(tv_obj);
        }
        break;

        case _TV_RowCol_ID(1, 1):
        {
            lv_watchface_layout_create(tv_obj);
        }
        break;

        case _TV_RowCol_ID(1, 2):
        {
            lv_heartrate_chart_layout_create(tv_obj);
        }
        break;

        case _TV_RowCol_ID(2, 1):
        {
            lv_notification_layout_create(tv_obj);
        }
        break;
    }
}

void lv_layout_tv_unload_win(uint8_t row_id, uint8_t col_id, lv_obj_t * tv_obj) {

}


volatile bool is_lvgl_tileview_scrolling = false;

static void lv_layout_tv_scroll_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    switch(code) {
        case LV_EVENT_SCROLL_BEGIN:
        {
            is_lvgl_tileview_scrolling = true;
        }
        break;

        case LV_EVENT_SCROLL:
        {
        }
        break;

        case LV_EVENT_SCROLL_END:
        {
            is_lvgl_tileview_scrolling = false;
        }
        break;

        default:break;
    }
}

bool lv_is_tileview_scrolling(void) {
    return is_lvgl_tileview_scrolling;
}

//override the lv_timer_is_filtered
bool lv_timer_is_filtered(lv_timer_t * timer) {
#if LV_GDX_PATCH_TIMER_PRIO > 0u
    if(is_lvgl_tileview_scrolling && (timer->sys_prio == 0)) {
        return true;
    }
#endif
    return false;
}
#endif
