#ifndef BSP_LCD_H
#define BSP_LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_types.h"

#define LCD_H_RES        320
#define LCD_V_RES        480
#define LCD_BPP          16
#define LCD_BYTES_PER_PIXEL (LCD_BPP / 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 LCD 硬件
 */
esp_err_t bsp_lcd_init(void);

/**
 * @brief 获取面板句柄
 */
esp_lcd_panel_handle_t bsp_lcd_get_panel(void);

/**
 * @brief 获取 I80 IO 句柄
 */
esp_lcd_panel_io_handle_t bsp_lcd_get_io(void);

/**
 * @brief 绘制位图
 */
esp_err_t bsp_lcd_draw_bitmap(uint16_t x_start, uint16_t y_start,
                              uint16_t x_end, uint16_t y_end,
                              const void *color_data);

/**
 * @brief 填充矩形区域（纯色）
 */
void bsp_lcd_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief 全屏清屏
 */
void bsp_lcd_clear(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // BSP_LCD_H