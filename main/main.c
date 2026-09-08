#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp_exio.h"
#include "lvgl_init.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

/**
 * @brief 挂载只读 FATFS 分区到 /storage
 * @return ESP_OK 成功，其他为失败
 */
esp_err_t mount_storage_fatfs(void) {
    const char *mount_point = "/storage";
    const char *partition_label = "storage";

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,   // 只读，不允许格式化
        .max_files = 5,
        .allocation_unit_size = 0,
    };

    esp_err_t err = esp_vfs_fat_spiflash_mount_ro(mount_point, partition_label, &mount_config);
    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "FATFS already mounted on %s", mount_point);
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "FATFS mounted on %s", mount_point);
    return ESP_OK;
}

/**
 * @brief 
 * @return 
 */
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

    // 
    mount_storage_fatfs();

    // 初始化 LVGL（内部会调用 bsp_lcd_init）
    ret = lvgl_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_init failed");
        return;
    }
    ESP_LOGI(TAG, "LVGL initialized");

    // 创建 LVGL 界面
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // 只创建一个图片对象（只做一次）
    lv_obj_t *img = lv_img_create(scr);
    lv_obj_center(img);  // 图片尺寸 320x480，无需设置大小，居中即可

    // 进入无限循环，只切换图片源，然后强制刷新
    int idx = 0;
    char path[32];

    while (1) {
        snprintf(path, sizeof(path), "S:/test/%04d.jpg", idx);
        lv_img_set_src(img, path);  // 更新图片源
        lv_refr_now(NULL);          // 强制立即刷新
        idx = (idx + 1) % 136;
    }

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}