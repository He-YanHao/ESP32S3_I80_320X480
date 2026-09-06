#ifndef BSP_EXIO_H
#define BSP_EXIO_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化板载 EXIO 端口 (XL9555)
 */
esp_err_t bsp_exio_port_init(void);

// 通用引脚控制
esp_err_t set_exio_out_mode(uint8_t pin);
esp_err_t set_exio_in_mode(uint8_t pin);
esp_err_t set_exio_out_up(uint8_t pin);
esp_err_t set_exio_out_down(uint8_t pin);
esp_err_t get_exio_in(uint8_t pin, bool *level);

// 功能专用控制 (P0.0~P0.4)
esp_err_t set_tp_rst_up(void);
esp_err_t set_tp_rst_down(void);
esp_err_t set_lcd_rst_up(void);
esp_err_t set_lcd_rst_down(void);
esp_err_t set_sd_cs_up(void);
esp_err_t set_sd_cs_down(void);
esp_err_t set_bl_ctr_up(void);
esp_err_t set_bl_ctr_down(void);
esp_err_t set_led1_up(void);
esp_err_t set_led1_down(void);

#ifdef __cplusplus
}
#endif

#endif // BSP_EXIO_H