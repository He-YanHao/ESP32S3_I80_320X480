#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化 LVGL 显示驱动
 * 固定分辨率 320x480，静态缓冲区，不含触摸
 */
esp_err_t lvgl_init(void);

/**
 * 获取 LVGL 显示驱动句柄
 */
lv_disp_t *lvgl_get_display(void);

/**
 * 获取 LVGL 显示缓冲区句柄
 */
lv_disp_draw_buf_t *lvgl_get_disp_buf(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_INIT_H