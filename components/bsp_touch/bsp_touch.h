#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 单点触摸数据结构（LVGL 8.x 仅支持单点）
 */
typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
} bsp_touch_point_t;

/**
 * @brief 初始化 GT1151Q 触摸屏（裸 I2C 驱动）
 * @note 调用前请确保 bsp_exio_port_init() 已执行（用于控制 TP_RST）
 */
esp_err_t bsp_touch_init(void);

/**
 * @brief 读取当前触摸点
 * @param[out] point 触摸数据，pressed 指示是否按下
 */
esp_err_t bsp_touch_read(bsp_touch_point_t *point);

#ifdef __cplusplus
}
#endif

#endif // BSP_TOUCH_H