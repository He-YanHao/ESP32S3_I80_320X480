#include "lvgl_init.h"
#include "bsp_lcd.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#define DRAW_BUF_LINES  60
#define BUF_SIZE_PIXELS (LCD_H_RES * DRAW_BUF_LINES)
#define BUF_SIZE_BYTES  (BUF_SIZE_PIXELS * sizeof(lv_color_t))

static const char *TAG = "LVGL_INIT";

// 显示缓冲区指针（从 PSRAM 分配）
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// LVGL v8.4.0 显示驱动变量
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_disp_t *disp_handle = NULL;

// LVGL 刷新回调
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_err_t ret = bsp_lcd_draw_bitmap(
        area->x1,
        area->y1,
        area->x2 + 1,
        area->y2 + 1,
        (uint8_t *)color_map
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_lcd_draw_bitmap failed: %d", ret);
    }

    // 直接通知 LVGL 刷新完成
    lv_disp_flush_ready(drv);
}

esp_err_t lvgl_init(void)
{
    esp_err_t ret;

    // 初始化 LCD 硬件
    ret = bsp_lcd_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_lcd_init failed");
        return ret;
    }
    ESP_LOGI(TAG, "LCD initialized");

    // 初始化 LVGL 核心
    lv_init();

    // 从 PSRAM 分配显示缓冲区
    size_t buffer_bytes = BUF_SIZE_PIXELS * sizeof(lv_color_t);
    buf1 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (buf1 == NULL || buf2 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate draw buffers from PSRAM");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "Draw buffers allocated from PSRAM: %zu bytes each", buffer_bytes);

    // 初始化显示缓冲区
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, BUF_SIZE_PIXELS);

    // 初始化显示驱动
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.rotated = 0;

    // 注册显示驱动
    disp_handle = lv_disp_drv_register(&disp_drv);
    if (disp_handle == NULL) {
        ESP_LOGE(TAG, "lv_disp_drv_register failed");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "LVGL initialized");
    return ESP_OK;
}

lv_disp_t *lvgl_get_display(void)
{
    return disp_handle;
}

lv_disp_draw_buf_t *lvgl_get_disp_buf(void)
{
    return &disp_buf;
}