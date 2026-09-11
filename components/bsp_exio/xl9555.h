#ifndef XL9555_H
#define XL9555_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 XL9555
 * @return ESP_OK 成功
 */
esp_err_t xl9555_init(void);

/**
 * @brief 配置引脚方向
 * @param port 0 或 1
 * @param pin 0~7
 * @param is_output true:输出, false:输入
 */
esp_err_t xl9555_set_pin_mode(uint8_t port, uint8_t pin, bool is_output);

/**
 * @brief 写入引脚电平
 * @param port 0 或 1
 * @param pin 0~7
 * @param level true:高, false:低
 */
esp_err_t xl9555_write_pin(uint8_t port, uint8_t pin, bool level);

/**
 * @brief 读取引脚电平
 * @param port 0 或 1
 * @param pin 0~7
 * @param level 输出参数
 */
esp_err_t xl9555_read_pin(uint8_t port, uint8_t pin, bool *level);

#ifdef __cplusplus
}
#endif

#endif // XL9555_H