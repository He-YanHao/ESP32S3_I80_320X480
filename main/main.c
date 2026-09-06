#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp_exio.h"
#include "bsp_lcd.h"

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

    // 初始化 LCD
    ret = bsp_lcd_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_lcd_init failed");
        return;
    }
    ESP_LOGI(TAG, "LCD initialized");

    // 先清屏为黑色
    bsp_lcd_clear(0x0000);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 四个角落彩块 (50x50)
    // 左上 (0,0) 红色
    bsp_lcd_fill_rect(0, 0, 49, 49, 0xF800);
    // 右上 (320-50, 0) 绿色
    bsp_lcd_fill_rect(270, 0, 319, 49, 0x07E0);
    // 左下 (0, 480-50) 蓝色
    bsp_lcd_fill_rect(0, 430, 49, 479, 0x001F);
    // 右下 (320-50, 480-50) 白色
    bsp_lcd_fill_rect(270, 430, 319, 479, 0xFFFF);

    // 中心黄色十字
    // 水平条 (中心y=240, 高20, 宽100)
    bsp_lcd_fill_rect(110, 230, 209, 249, 0xFFE0);
    // 垂直条 (中心x=160, 宽20, 高100)
    bsp_lcd_fill_rect(150, 190, 169, 289, 0xFFE0);

    ESP_LOGI(TAG, "Test pattern drawn. Check corners and center cross.");
    ESP_LOGI(TAG, "Expected: Red=TopLeft, Green=TopRight, Blue=BottomLeft, White=BottomRight");

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}