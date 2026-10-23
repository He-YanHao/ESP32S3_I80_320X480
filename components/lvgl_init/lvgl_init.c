#include "lvgl_init.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "bsp_exio.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

/* 定义全局 UI 结构体 */
lv_ui guider_ui;

static const char *TAG = "LVGL_INIT";

/* 全局递归互斥锁，保护 LVGL 端口的线程安全 */
static SemaphoreHandle_t lvgl_mutex = NULL;

/* 显示缓冲区指针（从 PSRAM 分配） */
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

/* LVGL 显示驱动变量 */
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_disp_t *disp_handle = NULL;

/* LVGL 触摸输入设备变量 */
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev_handle = NULL;

/* ============ LVGL 刷新回调 ============ */
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

    lv_disp_flush_ready(drv);
}

/* ============ LVGL 触摸读取回调 ============ */
static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    bsp_touch_point_t pt;
    if (bsp_touch_read(&pt) == ESP_OK && pt.pressed) {
        data->point.x = pt.x;
        data->point.y = pt.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* ============ 互斥锁 ============ */
bool lvgl_lock(void)
{
    if (lvgl_mutex == NULL) return false;
    return xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY) == pdTRUE;
}

bool lvgl_lock_timeout(uint32_t timeout_ms)
{
    if (lvgl_mutex == NULL) return false;
    return xSemaphoreTakeRecursive(lvgl_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void lvgl_unlock(void)
{
    if (lvgl_mutex) {
        xSemaphoreGiveRecursive(lvgl_mutex);
    }
}

/* ============ LVGL 主循环任务 ============ */
static void lvgl_task(void *arg)
{
    uint32_t task_delay_ms = LVGL_MIN_SLEEP_MS;
    ESP_LOGI(TAG, "Starting LVGL task");

    /* 初始化 UI 界面 */
    lvgl_lock();
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);
    lvgl_unlock();

    while (1) {
        lvgl_lock();
        task_delay_ms = lv_timer_handler();
        lvgl_unlock();

        if (task_delay_ms > LVGL_MAX_SLEEP_MS) {
            task_delay_ms = LVGL_MAX_SLEEP_MS;
        } else if (task_delay_ms < LVGL_MIN_SLEEP_MS) {
            task_delay_ms = LVGL_MIN_SLEEP_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }

    ESP_LOGI(TAG, "Stopped LVGL task");
    vTaskDelete(NULL);
}

/* ============ LVGL 初始化 ============ */
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

    // 初始化触摸屏
    ret = bsp_touch_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_touch_init failed");
        return ret;
    }
    ESP_LOGI(TAG, "Touch initialized");

    // 创建 LVGL 递归互斥锁
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    if (lvgl_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL recursive mutex");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "LVGL recursive mutex created");

    // 初始化 LVGL 核心
    lv_init();

    // 从 PSRAM 分配显示缓冲区
    size_t buffer_bytes = LCD_H_RES * LCD_V_RES * sizeof(lv_color_t);
    buf1 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (buf1 == NULL || buf2 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate draw buffers from PSRAM");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "Draw buffers allocated: %zu bytes each", buffer_bytes);

    // 初始化并注册显示驱动
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * LCD_V_RES);

    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.rotated = 0;

    disp_handle = lv_disp_drv_register(&disp_drv);
    if (disp_handle == NULL) {
        ESP_LOGE(TAG, "lv_disp_drv_register failed");
        return ESP_ERR_INVALID_STATE;
    }

    // 初始化并注册触摸输入设备
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read_cb;
    indev_handle = lv_indev_drv_register(&indev_drv);
    if (indev_handle == NULL) {
        ESP_LOGE(TAG, "lv_indev_drv_register failed");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "Touch input device registered");

    // 创建 LVGL 任务
    xTaskCreatePinnedToCoreWithCaps(
        lvgl_task,
        "lvgl_task",
        32U * 1024U,
        NULL,
        2,
        NULL,
        1,
        MALLOC_CAP_INTERNAL | MALLOC_CAP_DEFAULT
    );

    ESP_LOGI(TAG, "LVGL initialized");
    return ESP_OK;
}