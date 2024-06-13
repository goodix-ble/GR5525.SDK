#ifndef __DISPLAY_ST77916_PSD_360P_QSPI_DRV_H__
#define __DISPLAY_ST77916_PSD_360P_QSPI_DRV_H__

#include "app_qspi.h"
#include <stdint.h>

void display_st77916_psd_360p_init(app_qspi_id_t id, uint32_t clock_prescaler, app_qspi_evt_handler_t qspi_evt_handler);
void display_st77916_psd_360p_set_show_area(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
void display_st77916_psd_360p_wait_te_signal(void);

#endif // __DISPLAY_ST77916_PSD_360P_QSPI_DRV_H__
