#include "bsp_lcd.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7796.h"
#include "bsp_exio.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

static const char *TAG = "BSP_LCD";

// 引脚定义
#define LCD_CS_GPIO      GPIO_NUM_39
#define LCD_DC_GPIO      GPIO_NUM_38
#define LCD_WR_GPIO      GPIO_NUM_45

#define LCD_DATA0_GPIO   GPIO_NUM_13
#define LCD_DATA1_GPIO   GPIO_NUM_12
#define LCD_DATA2_GPIO   GPIO_NUM_11
#define LCD_DATA3_GPIO   GPIO_NUM_10
#define LCD_DATA4_GPIO   GPIO_NUM_9
#define LCD_DATA5_GPIO   GPIO_NUM_46
#define LCD_DATA6_GPIO   GPIO_NUM_3
#define LCD_DATA7_GPIO   GPIO_NUM_8
#define LCD_DATA8_GPIO   GPIO_NUM_18
#define LCD_DATA9_GPIO   GPIO_NUM_17
#define LCD_DATA10_GPIO  GPIO_NUM_16
#define LCD_DATA11_GPIO  GPIO_NUM_15
#define LCD_DATA12_GPIO  GPIO_NUM_7
#define LCD_DATA13_GPIO  GPIO_NUM_6
#define LCD_DATA14_GPIO  GPIO_NUM_5
#define LCD_DATA15_GPIO  GPIO_NUM_4

// DMA 传输限制 (100行，约64KB)
#define MAX_TRANSFER_ROWS LCD_V_RES
#define MAX_TRANSFER_BYTES (LCD_H_RES * MAX_TRANSFER_ROWS * LCD_BYTES_PER_PIXEL)

static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static esp_lcd_panel_handle_t s_panel_handle = NULL;

static void bsp_config_rd_pin(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_14,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_14, 1);
}

esp_err_t bsp_lcd_init(void)
{
    esp_err_t ret;
    esp_lcd_i80_bus_handle_t i80_bus = NULL;

    bsp_config_rd_pin();

    // 硬件复位
    set_lcd_rst_up();
    vTaskDelay(pdMS_TO_TICKS(10));
    set_lcd_rst_down();
    vTaskDelay(pdMS_TO_TICKS(50));
    set_lcd_rst_up();
    vTaskDelay(pdMS_TO_TICKS(200));

    // 创建 I80 总线
    esp_lcd_i80_bus_config_t bus_config = ST7796_PANEL_BUS_I80_CONFIG(
        MAX_TRANSFER_BYTES,
        16,
        LCD_DC_GPIO,
        LCD_WR_GPIO,
        LCD_DATA0_GPIO,  LCD_DATA1_GPIO,  LCD_DATA2_GPIO,  LCD_DATA3_GPIO,
        LCD_DATA4_GPIO,  LCD_DATA5_GPIO,  LCD_DATA6_GPIO,  LCD_DATA7_GPIO,
        LCD_DATA8_GPIO,  LCD_DATA9_GPIO,  LCD_DATA10_GPIO, LCD_DATA11_GPIO,
        LCD_DATA12_GPIO, LCD_DATA13_GPIO, LCD_DATA14_GPIO, LCD_DATA15_GPIO
    );
    ret = esp_lcd_new_i80_bus(&bus_config, &i80_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_new_i80_bus failed");
        return ret;
    }

    // 创建 IO 设备
    esp_lcd_panel_io_i80_config_t io_config = ST7796_PANEL_IO_I80_CONFIG(
        LCD_CS_GPIO,
        NULL,
        NULL
    );
    ret = esp_lcd_new_panel_io_i80(i80_bus, &io_config, &s_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_new_panel_io_i80 failed");
        return ret;
    }

    // 创建 ST7796 面板
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BPP,
    };
    ret = esp_lcd_new_panel_st7796(s_io_handle, &panel_config, &s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_new_panel_st7796 failed");
        return ret;
    }

    // 初始化面板
    esp_lcd_panel_reset(s_panel_handle);
    esp_lcd_panel_init(s_panel_handle);
    esp_lcd_panel_invert_color(s_panel_handle, true);
    esp_lcd_panel_set_gap(s_panel_handle, 0, 0);
    esp_lcd_panel_disp_on_off(s_panel_handle, true);
    esp_lcd_panel_mirror(s_panel_handle, true, false);

    // 打开背光
    set_bl_ctr_up();

    ESP_LOGI(TAG, "LCD initialized");
    return ESP_OK;
}

esp_lcd_panel_handle_t bsp_lcd_get_panel(void)
{
    return s_panel_handle;
}

esp_lcd_panel_io_handle_t bsp_lcd_get_io(void)
{
    return s_io_handle;
}

esp_err_t bsp_lcd_draw_bitmap(uint16_t x_start, uint16_t y_start,
                              uint16_t x_end, uint16_t y_end,
                              const void *color_data)
{
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t height = y_end - y_start;
    if (height <= MAX_TRANSFER_ROWS) {
        return esp_lcd_panel_draw_bitmap(s_panel_handle, x_start, y_start, x_end, y_end, color_data);
    }

    // 分块发送
    uint16_t y_pos = y_start;
    const uint8_t *data_ptr = (const uint8_t *)color_data;
    uint16_t width = x_end - x_start;
    uint32_t row_bytes = width * LCD_BYTES_PER_PIXEL;

    while (y_pos < y_end) {
        uint16_t rows_this = (y_pos + MAX_TRANSFER_ROWS > y_end) ? (y_end - y_pos) : MAX_TRANSFER_ROWS;
        esp_err_t ret = esp_lcd_panel_draw_bitmap(s_panel_handle,
                                                  x_start, y_pos,
                                                  x_end, y_pos + rows_this,
                                                  data_ptr);
        if (ret != ESP_OK) return ret;
        data_ptr += rows_this * row_bytes;
        y_pos += rows_this;
        vTaskDelay(1);
    }
    return ESP_OK;
}

void bsp_lcd_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (s_panel_handle == NULL) return;

    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;

    uint16_t *line_buf = (uint16_t *)heap_caps_malloc(width * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (line_buf == NULL) {
        line_buf = (uint16_t *)heap_caps_malloc(width * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (line_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate line buffer");
        return;
    }

    for (uint16_t i = 0; i < width; i++) {
        line_buf[i] = color;
    }

    for (uint16_t y = 0; y < height; y++) {
        bsp_lcd_draw_bitmap(x1, y1 + y, x2 + 1, y1 + y + 1, line_buf);
    }
    free(line_buf);
}

void bsp_lcd_clear(uint16_t color)
{
    bsp_lcd_fill_rect(0, 0, LCD_H_RES - 1, LCD_V_RES - 1, color);
}