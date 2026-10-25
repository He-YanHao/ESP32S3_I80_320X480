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
/* I2C 单次传输超时：不能用 -1（永久等待），否则总线异常会冻结调用它的 LVGL 任务 */
#define TOUCH_I2C_TIMEOUT_MS    50
/* 总线恢复的最小间隔，避免每次读取失败都去复位总线 */
#define TOUCH_RECOVER_INTERVAL_MS 1000

/* ========== GT1151Q 寄存器 ========== */
#define GT1151_REG_PID      0x8140      /* 产品 ID */
#define GT1151_REG_STATUS   0x814E      /* 状态：bit7=1 坐标就绪 */
#define GT1151_REG_TP1      0x8150      /* 第一个触摸点坐标起始 */

/* ========== 静态句柄 ========== */
static i2c_master_bus_handle_t touch_bus = NULL;
static i2c_master_dev_handle_t touch_dev = NULL;
static TickType_t s_last_recover_tick = 0;
static bool s_has_recovered = false;

/* 通信失败时尝试恢复总线（限频），避免 I2C 卡死后再也无法读取触摸 */
static void touch_try_recover(void)
{
    if (touch_bus == NULL) {
        return;
    }
    TickType_t now = xTaskGetTickCount();
    if (s_has_recovered &&
            (now - s_last_recover_tick) < pdMS_TO_TICKS(TOUCH_RECOVER_INTERVAL_MS)) {
        return;
    }
    s_has_recovered = true;
    s_last_recover_tick = now;

    esp_err_t err = i2c_master_bus_reset(touch_bus);
    if (err == ESP_OK) {
        ESP_LOGW(TAG, "I2C bus reset after touch read error");
    } else {
        ESP_LOGE(TAG, "I2C bus reset failed: %s", esp_err_to_name(err));
    }
}

/* ========== I2C 读写底层 ========== */
static esp_err_t touch_write_reg(uint16_t reg, const uint8_t *data, size_t len)
{
    uint8_t buf[2 + len];
    buf[0] = (uint8_t)(reg >> 8);
    buf[1] = (uint8_t)(reg & 0xFF);
    if (len > 0 && data != NULL) {
        memcpy(&buf[2], data, len);
    }
    return i2c_master_transmit(touch_dev, buf, 2 + len, TOUCH_I2C_TIMEOUT_MS);
}

static esp_err_t touch_read_reg(uint16_t reg, uint8_t *data, size_t len)
{
    uint8_t addr[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF) };
    return i2c_master_transmit_receive(touch_dev, addr, 2, data, len, TOUCH_I2C_TIMEOUT_MS);
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

    /* 4. 读取产品 ID 验证通信（失败重试几次，仍失败说明触摸不可用） */
    uint8_t pid[4] = {0};
    esp_err_t pid_err = ESP_FAIL;
    for (int i = 0; i < 3; i++) {
        pid_err = touch_read_reg(GT1151_REG_PID, pid, 4);
        if (pid_err == ESP_OK) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    if (pid_err == ESP_OK) {
        ESP_LOGI(TAG, "Touch IC ID: %c%c%c%c", pid[0], pid[1], pid[2], pid[3]);
    } else {
        /* 注意：这里不返回错误，避免整个 UI 起不来；但触摸会不可用，必须能从日志看出来 */
        ESP_LOGE(TAG, "Failed to read Touch IC ID (%s), touch will not work",
                 esp_err_to_name(pid_err));
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
        touch_try_recover();
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