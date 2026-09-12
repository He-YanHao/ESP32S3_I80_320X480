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

// 
static const char *TAG = "LVGL_INIT";

// 全局递归互斥锁句柄 用于保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
static SemaphoreHandle_t lvgl_mutex = NULL;

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

/**
 * @brief 锁定 LVGL 端口以确保线程安全
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreTakeRecursive() 来获取递归互斥锁，等待时间设置为 portMAX_DELAY 表示无限等待，直到成功获取锁
 * 3. 在调用该函数之前应确保递归互斥锁已经成功创建，否则可能会导致未定义行为
 * @return true 成功获取锁，false 获取锁失败
 */
bool lvgl_lock(void)
{
    // 如果递归互斥锁未创建，直接返回 false 表示获取锁失败
    if (lvgl_mutex == NULL) return false;
    // 调用 xSemaphoreTakeRecursive() 来获取递归互斥锁，等待时间设置为 portMAX_DELAY 表示无限等待，直到成功获取锁
    return xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY) == pdTRUE;
}

/**
 * @brief 带超时的锁定 LVGL 端口函数
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreTakeRecursive() 来获取递归互斥锁，等待时间根据传入的 timeout_ms 参数来设置，单位为毫秒
 * 3. 在调用该函数之前应确保递归互斥锁已经成功创建，否则可能会导致未定义行为
 * @param timeout_ms 等待时间，单位为毫秒，设置为 0 表示不等待直接返回
 */
bool lvgl_lock_timeout(uint32_t timeout_ms)
{
    // 如果递归互斥锁未创建，直接返回 false 表示获取锁失败
    if (lvgl_mutex == NULL) return false;
    // 根据传入的 timeout_ms 参数来设置等待时间。
    return xSemaphoreTakeRecursive(lvgl_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

/**
 * @brief 解锁 LVGL 端口
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreGiveRecursive() 来释放递归互斥锁，允许其他任务获取锁并访问 LVGL 端口
 * 3. 在调用该函数之前应确保当前任务已经成功获取锁，否则可能会导致未定义行为
 */
void lvgl_unlock(void)
{
    // 如果递归互斥锁未创建，直接返回，不执行解锁操作
    if (lvgl_mutex) {
        xSemaphoreGiveRecursive(lvgl_mutex);
    }
}

/**
 * @brief LVGL 主循环任务函数
 * @param arg 任务参数（未使用）
 */
static void lvgl_task(void *arg)
{
    uint32_t task_delay_ms = LVGL_MIN_SLEEP_MS;
    ESP_LOGI(TAG, "Starting My LVGL task");

    // 初始化 UI 界面
    lvgl_lock();

    // 


    lvgl_unlock();

    while (1) {
        // 尝试锁定 LVGL 端口，如果成功则调用 lv_timer_handler() 来处理 LVGL 的心跳，并根据返回的建议延时调整 task_delay_ms 的值
        lvgl_lock();
        task_delay_ms = lv_timer_handler();
        lvgl_unlock();

        // 限制休眠时间范围：[LVGL_MIN_SLEEP_MS ms, LVGL_MAX_SLEEP_MS ms]
        // 确保既不会过于频繁地调用 ui_tick() 导致性能问题，也能保持界面响应及时和流畅
        if (task_delay_ms > LVGL_MAX_SLEEP_MS) {
            task_delay_ms = LVGL_MAX_SLEEP_MS;
        } else if (task_delay_ms < LVGL_MIN_SLEEP_MS) {
            task_delay_ms = LVGL_MIN_SLEEP_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
    // 后续会增加LVGL注销事件
    ESP_LOGI(TAG, "Stopped LVGL task");
    // 最后调用 vTaskDelete(NULL) 来删除当前任务，释放其占用的资源，并结束任务的执行。
    vTaskDelete( NULL );
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

    // 创建一个递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题。
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    if (lvgl_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL recursive mutex");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "LVGL recursive mutex created successfully");

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

    // 创建一个新的 FreeRTOS 任务来运行 lvgl_task，该任务将负责调用 ui_tick() 来处理 UI 更新。
    xTaskCreatePinnedToCoreWithCaps(lvgl_task,    // 任务函数
                            "lvgl_task",          // 任务名称
                            8192,                 // 栈大小（字节）
                            NULL,                 // 任务参数
                            2,                    // 优先级
                            NULL,                 // 任务句柄（不需要可传 NULL）
                            1,                    // 核心编号：0 = CPU0, 1 = CPU1
                            MALLOC_CAP_INTERNAL | MALLOC_CAP_DEFAULT // 内存能力：确保分配到内部内存以获得更好的性能
                        ); 

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