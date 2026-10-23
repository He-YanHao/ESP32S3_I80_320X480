#include "bsp_touch.h"
#include "bsp_exio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "bsp_touch";

/* ========== 硬件配置 ========== */
#define TOUCH_I2C_PORT      I2C_NUM_1
#define TOUCH_SCL_PIN       GPIO_NUM_40
#define TOUCH_SDA_PIN       GPIO_NUM_41
#define TOUCH_I2C_SPEED     400000
#define TOUCH_DEV_ADDR      0x14

/* ========== GT1151Q 寄存器 ========== */
#define GT1151_REG_PID      0x8140      /* 产品 ID */
#define GT1151_REG_STATUS   0x814E      /* 状态：bit7=1 坐标就绪 */
#define GT1151_REG_TP1      0x8150      /* 第一个触摸点坐标起始 */

/* ========== 静态句柄 ========== */
static i2c_master_bus_handle_t touch_bus = NULL;
static i2c_master_dev_handle_t touch_dev = NULL;

/* ========== I2C 读写底层 ========== */
static esp_err_t touch_write_reg(uint16_t reg, const uint8_t *data, size_t len)
{
    uint8_t buf[2 + len];
    buf[0] = (uint8_t)(reg >> 8);
    buf[1] = (uint8_t)(reg & 0xFF);
    if (len > 0 && data != NULL) {
        memcpy(&buf[2], data, len);
    }
    return i2c_master_transmit(touch_dev, buf, 2 + len, -1);
}

static esp_err_t touch_read_reg(uint16_t reg, uint8_t *data, size_t len)
{
    uint8_t addr[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF) };
    return i2c_master_transmit_receive(touch_dev, addr, 2, data, len, -1);
}

/* ========== 初始化 ========== */
esp_err_t bsp_touch_init(void)
{
    /* 1. 创建 I2C 主机总线 */
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = TOUCH_I2C_PORT,
        .scl_io_num = TOUCH_SCL_PIN,
        .sda_io_num = TOUCH_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &touch_bus), TAG, "i2c bus init failed");

    /* 2. 挂载 GT1151Q */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TOUCH_DEV_ADDR,
        .scl_speed_hz = TOUCH_I2C_SPEED,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(touch_bus, &dev_cfg, &touch_dev),
                        TAG, "add device failed");

    /* 3. 通过 XL9555 硬件复位 GT1151Q */
    set_tp_rst_down();
    vTaskDelay(pdMS_TO_TICKS(20));
    set_tp_rst_up();
    vTaskDelay(pdMS_TO_TICKS(300));

    /* 4. 读取产品 ID 验证通信 */
    uint8_t pid[4] = {0};
    if (touch_read_reg(GT1151_REG_PID, pid, 4) == ESP_OK) {
        ESP_LOGI(TAG, "Touch IC ID: %c%c%c%c", pid[0], pid[1], pid[2], pid[3]);
    } else {
        ESP_LOGW(TAG, "Failed to read Touch IC ID");
    }

    ESP_LOGI(TAG, "Touch initialized");
    return ESP_OK;
}

/* ========== 读取单点坐标 ========== */
esp_err_t bsp_touch_read(bsp_touch_point_t *point)
{
    if (point == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    point->x = 0;
    point->y = 0;
    point->pressed = false;

    uint8_t status = 0;
    esp_err_t ret = touch_read_reg(GT1151_REG_STATUS, &status, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    /* bit7=0 表示坐标未就绪 */
    if ((status & 0x80) == 0) {
        return ESP_OK;
    }

    uint8_t touch_cnt = status & 0x0F;
    if (touch_cnt > 0) {
        uint8_t buf[4] = {0};
        ret = touch_read_reg(GT1151_REG_TP1, buf, 4);
        if (ret == ESP_OK) {
            point->x = ((uint16_t)buf[1] << 8) | buf[0];
            point->y = ((uint16_t)buf[3] << 8) | buf[2];
            point->pressed = true;
        }
    }

    /* 读完后必须清 0x814E，否则下一帧不会更新 */
    uint8_t clear = 0;
    touch_write_reg(GT1151_REG_STATUS, &clear, 1);

    return ESP_OK;
}