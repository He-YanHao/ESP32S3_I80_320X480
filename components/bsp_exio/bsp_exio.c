#include "bsp_exio.h"
#include "xl9555.h"
#include "esp_log.h"

static const char *TAG = "bsp_exio";

static bool s_exio_inited = false;

esp_err_t bsp_exio_port_init(void)
{
    if (s_exio_inited) {
        return ESP_OK;
    }

    esp_err_t err = xl9555_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "xl9555_init failed");
        return err;
    }

    // 配置固定功能引脚为输出
    xl9555_set_pin_mode(0, 0, true);   // TP_RST
    xl9555_set_pin_mode(0, 1, true);   // LCD_RST
    xl9555_set_pin_mode(0, 2, true);   // SD_CS
    xl9555_set_pin_mode(0, 3, true);   // BL_CTR
    xl9555_set_pin_mode(0, 4, true);   // LED1

    s_exio_inited = true;
    ESP_LOGI(TAG, "EXIO pins initialized");
    return ESP_OK;
}

esp_err_t set_exio_out_mode(uint8_t pin)
{
    return xl9555_set_pin_mode(pin / 8, pin % 8, true);
}

esp_err_t set_exio_in_mode(uint8_t pin)
{
    return xl9555_set_pin_mode(pin / 8, pin % 8, false);
}

esp_err_t set_exio_out_up(uint8_t pin)
{
    return xl9555_write_pin(pin / 8, pin % 8, true);
}

esp_err_t set_exio_out_down(uint8_t pin)
{
    return xl9555_write_pin(pin / 8, pin % 8, false);
}

esp_err_t get_exio_in(uint8_t pin, bool *level)
{
    return xl9555_read_pin(pin / 8, pin % 8, level);
}

esp_err_t set_tp_rst_up(void)   { return xl9555_write_pin(0, 0, true); }
esp_err_t set_tp_rst_down(void) { return xl9555_write_pin(0, 0, false); }
esp_err_t set_lcd_rst_up(void)   { return xl9555_write_pin(0, 1, true); }
esp_err_t set_lcd_rst_down(void) { return xl9555_write_pin(0, 1, false); }
esp_err_t set_sd_cs_up(void)   { return xl9555_write_pin(0, 2, true); }
esp_err_t set_sd_cs_down(void) { return xl9555_write_pin(0, 2, false); }
esp_err_t set_bl_ctr_up(void)   { return xl9555_write_pin(0, 3, true); }
esp_err_t set_bl_ctr_down(void) { return xl9555_write_pin(0, 3, false); }
esp_err_t set_led1_up(void)   { return xl9555_write_pin(0, 4, true); }
esp_err_t set_led1_down(void) { return xl9555_write_pin(0, 4, false); }