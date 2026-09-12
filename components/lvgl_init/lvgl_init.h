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
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreTakeRecursive() 来获取递归互斥锁，等待时间设置为 portMAX_DELAY 表示无限等待，直到成功获取锁
 * 3. 在调用该函数之前应确保递归互斥锁已经成功创建，否则可能会导致未定义行为
 * @return true 成功获取锁，false 获取锁失败
 */
bool lvgl_lock(void);

/**
 * @brief 带超时的锁定 LVGL 端口函数
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreTakeRecursive() 来获取递归互斥锁，等待时间根据传入的 timeout_ms 参数来设置，单位为毫秒
 * 3. 在调用该函数之前应确保递归互斥锁已经成功创建，否则可能会导致未定义行为
 * @param timeout_ms 等待时间，单位为毫秒，设置为 0 表示不等待直接返回
 */
bool lvgl_lock_timeout(uint32_t timeout_ms);

/**
 * @brief 解锁 LVGL 端口
 * 1. 使用 FreeRTOS 的递归互斥锁来保护 LVGL 端口的访问，确保在多任务环境下对 LVGL 的调用是线程安全的，避免数据竞争和不一致问题
 * 2. 调用 xSemaphoreGiveRecursive() 来释放递归互斥锁，允许其他任务获取锁并访问 LVGL 端口
 * 3. 在调用该函数之前应确保当前任务已经成功获取锁，否则可能会导致未定义行为
 */
void lvgl_unlock(void);

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