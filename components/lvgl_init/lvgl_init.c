#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_init.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lv_adapter.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

// 引入你现有的 BSP 头文件
#include "bsp_lcd.h"
#include "bsp_touch.h"

static const char *TAG = "lvgl_init";

static esp_lcd_panel_handle_t s_panel = NULL;
static lv_display_t          *s_disp  = NULL;

// 局部刷新缓冲（与旧版 LVGL8 一致的 60 行，从 PSRAM 分配）
#define DRAW_BUF_LINES    60
#define BUF_SIZE_PIXELS   (LCD_H_RES * DRAW_BUF_LINES)
#define BUF_SIZE_BYTES    (BUF_SIZE_PIXELS * 2)   // RGB565

static void *buf1 = NULL;
static void *buf2 = NULL;

/*
 * 手动 flush 回调：直接把 LVGL 原生 RGB565 数据交给 esp_lcd_panel_draw_bitmap 发送，
 * 不做任何字节交换。
 *
 * 之所以不用 esp_lv_adapter_register_display()：adapter 对 "OTHER"(I80/SPI) + RGB565
 * 会在 flush 阶段无条件做一次 lv_draw_sw_rgb565_swap（软件字节交换），且没有关闭选项。
 * 而本屏的 16bit I80 总线需要原生 RGB565 字节序（纯色直刷测试已验证），多这一次交换
 * 会导致背景偏绿。因此这里绕开 adapter 的显示注册，手动注册显示 + 自定义 flush 回调。
 */
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_display_flush_ready(disp);
}

bool lvgl_lock(void)
{
    return esp_lv_adapter_lock(-1) == ESP_OK;
}

bool lvgl_lock_timeout(uint32_t timeout_ms)
{
    return esp_lv_adapter_lock((int32_t)timeout_ms) == ESP_OK;
}

void lvgl_unlock(void)
{
    esp_lv_adapter_unlock();
}

esp_err_t lvgl_init(void)
{
    esp_err_t ret;

    // 1. 初始化 LCD 硬件（I80 + ST7796）
    ESP_RETURN_ON_ERROR(bsp_lcd_init(), TAG, "bsp_lcd_init failed");
    s_panel = bsp_lcd_get_panel();
    ESP_RETURN_ON_FALSE(s_panel, ESP_ERR_INVALID_STATE, TAG, "LCD handle null");

    // 2. 初始化触摸硬件
    ESP_RETURN_ON_ERROR(bsp_touch_init(), TAG, "bsp_touch_init failed");

    // 3. 初始化 esp_lv_adapter（负责 lv_init、互斥锁、tick 定时器、LVGL 渲染任务）
    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    cfg.task_core_id = 1;
    ESP_RETURN_ON_ERROR(esp_lv_adapter_init(&cfg), TAG, "adapter init failed");

    // 4. 手动注册显示设备（不使用 esp_lv_adapter_register_display，避免其软件字节交换）
    buf1 = heap_caps_malloc(BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = heap_caps_malloc(BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(buf1 && buf2, ESP_ERR_NO_MEM, TAG, "alloc draw buffers failed");

    s_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    ESP_RETURN_ON_FALSE(s_disp, ESP_ERR_NO_MEM, TAG, "display create failed");
    lv_display_set_flush_cb(s_disp, lvgl_flush_cb);
    lv_display_set_buffers(s_disp, buf1, buf2, BUF_SIZE_BYTES, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 5. 启动适配器任务（内部会周期调用 lv_timer_handler）
    ESP_RETURN_ON_ERROR(esp_lv_adapter_start(), TAG, "adapter start failed");

    ESP_LOGI(TAG, "LVGL init ok (manual display, I80)");
    return ESP_OK;
}
