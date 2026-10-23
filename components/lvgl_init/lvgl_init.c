#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_init.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_lv_adapter.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

// 引入你现有的 BSP 头文件
#include "bsp_lcd.h"
#include "bsp_touch.h" 
// 注意：若要使用 esp_lv_adapter 的触摸注册，需要提供 esp_lcd_touch_handle_t
// 这里假设你已通过 esp_lcd_touch 组件初始化了 GT1151Q，或通过适配层封装好了句柄

static const char *TAG = "lvgl_init";

static esp_lcd_panel_handle_t    s_panel = NULL;
static esp_lcd_panel_io_handle_t s_io    = NULL;
static esp_lcd_touch_handle_t    s_touch = NULL; // 需要是 esp_lcd_touch 句柄
static lv_display_t              *s_disp  = NULL;

/* 
 * I80 在 esp_lvgl_adapter 中属于 "OTHER" 接口，不支持 TE 防撕裂，仅支持 NONE。
 * 同时适配器不对 90°/270° 做旋转处理，旋转必须在 LCD 初始化阶段配置面板方向。
 * 因此这里固定使用 ROTATE_0，并通过 bsp_lcd 内部（如 esp_lcd_panel_mirror / swap_xy）完成方向调整。
 */
#define ROTATION_ANGLE   ESP_LV_ADAPTER_ROTATE_0

// 根据屏幕实际物理分辨率设置（ST7796 通常为 320x480）
#define H_RES  LCD_H_RES
#define V_RES  LCD_V_RES

esp_err_t lvgl_init(void)
{
    esp_err_t ret;

    // 1. 初始化你的 LCD 硬件（I80 + ST7796）
    // bsp_lcd_init 内部已创建好 i80_bus、panel_io 和 panel
    ESP_RETURN_ON_ERROR(bsp_lcd_init(), TAG, "bsp_lcd_init failed");
    s_panel = bsp_lcd_get_panel();
    s_io    = bsp_lcd_get_io();
    ESP_RETURN_ON_FALSE(s_panel && s_io, ESP_ERR_INVALID_STATE, TAG, "LCD handles null");

    // 2. 初始化触摸硬件
    // 注意：bsp_touch_init 目前是裸 I2C 驱动，返回的不是 esp_lcd_touch_handle_t。
    // 若要使用 esp_lv_adapter 的触摸注册，需要额外通过 esp_lcd_touch 封装 GT1151Q。
    // 如果暂时不想改触摸驱动，可以注释掉这一段，改用旧版的手动注册 indev（见文末说明）。
    ESP_RETURN_ON_ERROR(bsp_touch_init(), TAG, "bsp_touch_init failed");
    // s_touch = ...; // 你需要从这里获取 esp_lcd_touch_handle_t

    // 3. 初始化 esp_lv_adapter
    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    cfg.task_core_id = 1; // 绑定到核心 1
    ESP_RETURN_ON_ERROR(esp_lv_adapter_init(&cfg), TAG, "adapter init failed");

    // 4. 注册显示设备
    // I80 属于 "OTHER" 接口，使用 SPI_WITH_PSRAM 或 SPI_WITHOUT_PSRAM 默认配置宏。
    // 由于旧版代码从 PSRAM 分配帧缓冲，这里使用 WITH_PSRAM 版本。
    esp_lv_adapter_display_config_t disp_cfg =
        ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_DEFAULT_CONFIG(
            s_panel,          // LCD panel handle
            s_io,             // LCD panel IO handle (I80 需要)
            H_RES,            // 水平分辨率
            V_RES,            // 垂直分辨率
            ROTATION_ANGLE    // 旋转角度（I80 仅支持 0/180，且建议在 LCD 内处理）
        );
    s_disp = esp_lv_adapter_register_display(&disp_cfg);
    ESP_RETURN_ON_FALSE(s_disp, ESP_ERR_NO_MEM, TAG, "register display failed");

    // 5. 注册触摸设备（可选，取决于你是否将 bsp_touch 封装为 esp_lcd_touch_handle_t）
    if (s_touch != NULL) {
        esp_lv_adapter_touch_config_t touch_cfg =
            ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(s_disp, s_touch);
        lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_cfg);
        ESP_RETURN_ON_FALSE(touch, ESP_ERR_NO_MEM, TAG, "register touch failed");
    } else {
        ESP_LOGW(TAG, "No esp_lcd_touch handle, skip adapter touch registration");
        // 如果跳过这里，你需要像旧版一样手动创建 lv_indev 并注册 read_cb
    }

    // 6. 启动适配器任务
    ESP_RETURN_ON_ERROR(esp_lv_adapter_start(), TAG, "adapter start failed");

    ESP_LOGI(TAG, "LVGL adapter init ok (I80/OTHER), rotation=%d", ROTATION_ANGLE);
    return ESP_OK;
}