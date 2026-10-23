#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "esp_err.h"
#include "lvgl.h"

#define LVGL_MAX_SLEEP_MS  50
#define LVGL_MIN_SLEEP_MS  5

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 锁定 LVGL 端口以确保线程安全
 */
bool lvgl_lock(void);

/**
 * @brief 带超时的锁定 LVGL 端口函数
 */
bool lvgl_lock_timeout(uint32_t timeout_ms);

/**
 * @brief 解锁 LVGL 端口
 */
void lvgl_unlock(void);

/**
 * @brief 初始化 LVGL 显示和触摸驱动
 * @note 内部会调用 bsp_lcd_init() 和 bsp_touch_init()，
 *       请确保在此函数之前调用 bsp_exio_port_init()
 */
esp_err_t lvgl_init(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_INIT_H