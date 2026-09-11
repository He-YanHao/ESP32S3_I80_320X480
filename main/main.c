#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp_exio.h"
#include "lvgl_init.h"
#include "esp_vfs_fat.h"
#include "lvgl.h"

static const char *TAG = "MAIN";

// 挂载只读 FATFS
esp_err_t mount_storage_fatfs(void) {
    const char *mount_point = "/storage";
    const char *partition_label = "storage";
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
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

void app_main(void) {
    esp_err_t ret;

    // 1. 初始化 EXIO
    ret = bsp_exio_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_exio_port_init failed");
        return;
    }

    // 2. 挂载 FATFS（VFS 挂载点 /storage）
    ret = mount_storage_fatfs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Mount FATFS failed");
        return;
    }

    // 3. 初始化 LVGL（包括 LCD 和 LVGL 内核）
    ret = lvgl_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_init failed");
        return;
    }
    ESP_LOGI(TAG, "LVGL initialized");

    // 5. 创建屏幕背景（黑色）
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // 6. 创建图片对象（固定用于显示）
    lv_obj_t *img = lv_img_create(scr);
    lv_obj_center(img);

    // 7. 准备图片路径（盘符 S: 对应 /storage）
    char path[32];
    const int num_files = 82;           // 根据实际文件数量（0000.sjpg ~ 0135.sjpg）
    int idx = 0;

    // 8. 主循环：无限切换图片
    while (1) {
        snprintf(path, sizeof(path), "S:/%04d.jpg", idx);
        lv_img_set_src(img, path);
        lv_timer_handler();
        // 等待一段时间（例如 500ms）再切换下一张
        vTaskDelay(pdMS_TO_TICKS(1));
        idx++;
        if (idx >= num_files) {
            idx = 0;
        }
    }
}