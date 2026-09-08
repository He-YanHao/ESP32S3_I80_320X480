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

// 显式注册 SJPG 解码器
static void register_sjpg_decoder(void) {
    extern void lv_split_jpeg_init(void);
    lv_split_jpeg_init();
    ESP_LOGI(TAG, "SJPG decoder initialized");
}

void app_main(void) {
    esp_err_t ret;

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // 1. 初始化 EXIO
    ret = bsp_exio_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_exio_port_init failed");
        return;
    }
    ESP_LOGI(TAG, "EXIO initialized");

    // 2. 挂载 FATFS
    ret = mount_storage_fatfs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Mount FATFS failed");
        return;
    }

    // 3. 检查文件头部（验证文件完整性）
    FILE *f_test = fopen("/storage/0000.sjpg", "rb");
    if (f_test) {
        unsigned char header[4];
        size_t n = fread(header, 1, 4, f_test);
        fclose(f_test);
        if (n == 4) {
            ESP_LOGI(TAG, "File header: %02X %02X %02X %02X", header[0], header[1], header[2], header[3]);
            if (header[0] == 0xFF && header[1] == 0xD8) {
                ESP_LOGI(TAG, "Valid JPEG/SJPG file");
            } else {
                ESP_LOGE(TAG, "Invalid JPEG header!");
            }
        } else {
            ESP_LOGE(TAG, "Failed to read header");
        }
    } else {
        ESP_LOGE(TAG, "Cannot open /storage/0000.sjpg");
    }

    // 4. 初始化 LVGL（内部调用 bsp_lcd_init 和 lv_init）
    ret = lvgl_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_init failed");
        return;
    }
    ESP_LOGI(TAG, "LVGL initialized");

    // 5. 显式注册 SJPG 解码器（保险）
    register_sjpg_decoder();

    // 6. 创建屏幕背景（黑色）
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // 7. 创建图片对象
    lv_obj_t *img = lv_img_create(scr);
    lv_obj_center(img);

    // 8. 尝试显示一张图片
    const char *path = "S:/test/0000.sjpg";
    ESP_LOGI(TAG, "Loading image: %s", path);

    lv_img_set_src(img, path);
    lv_obj_update_layout(img);
    lv_coord_t w = lv_obj_get_width(img);
    lv_coord_t h = lv_obj_get_height(img);
    ESP_LOGI(TAG, "Image size after set: %d x %d", w, h);

    if (w == 0 || h == 0) {
        ESP_LOGE(TAG, "Image decoding failed (size 0)");
    }

    lv_refr_now(NULL);

    // 9. 主循环（保持 LVGL 运行）
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}