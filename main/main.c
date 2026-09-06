#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp_exio.h"
#include "lvgl_init.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    esp_err_t ret;

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // 初始化 EXIO
    ret = bsp_exio_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_exio_port_init failed");
        return;
    }
    ESP_LOGI(TAG, "EXIO initialized");

    // 初始化 LVGL（内部会调用 bsp_lcd_init）
    ret = lvgl_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_init failed");
        return;
    }
    ESP_LOGI(TAG, "LVGL initialized");

    // 创建 LVGL 界面
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // 强制刷新显示
    lv_refr_now(NULL);

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}