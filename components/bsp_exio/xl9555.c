#include "xl9555.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "xl9555";

// 硬件配置
#define XL9555_I2C_PORT     I2C_NUM_0
#define XL9555_SCL_PIN      1
#define XL9555_SDA_PIN      2
#define XL9555_I2C_SPEED    400000
#define XL9555_DEV_ADDR     0x20

// 寄存器定义
#define REG_INPUT_PORT_0     0x00
#define REG_INPUT_PORT_1     0x01
#define REG_OUTPUT_PORT_0    0x02
#define REG_OUTPUT_PORT_1    0x03
#define REG_POLARITY_INV_0   0x04
#define REG_POLARITY_INV_1   0x05
#define REG_CONFIG_PORT_0    0x06
#define REG_CONFIG_PORT_1    0x07

static i2c_master_bus_handle_t bus_handle = NULL;
static i2c_master_dev_handle_t dev_handle = NULL;

static esp_err_t write_reg(uint8_t reg, uint8_t data)
{
    if (dev_handle == NULL) return ESP_ERR_INVALID_STATE;
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), -1);
}

static esp_err_t read_reg(uint8_t reg, uint8_t *data)
{
    if (dev_handle == NULL || data == NULL) return ESP_ERR_INVALID_STATE;
    esp_err_t err = i2c_master_transmit(dev_handle, &reg, 1, -1);
    if (err != ESP_OK) return err;
    return i2c_master_receive(dev_handle, data, 1, -1);
}

static esp_err_t modify_reg(uint8_t reg, uint8_t bit_mask, bool set)
{
    uint8_t reg_val;
    esp_err_t ret = read_reg(reg, &reg_val);
    if (ret != ESP_OK) return ret;
    if (set) reg_val |= bit_mask;
    else reg_val &= ~bit_mask;
    return write_reg(reg, reg_val);
}

esp_err_t xl9555_init(void)
{
    if (bus_handle != NULL) {
        ESP_LOGW(TAG, "XL9555 already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = XL9555_I2C_PORT,
        .scl_io_num = XL9555_SCL_PIN,
        .sda_io_num = XL9555_SDA_PIN,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed");
        return err;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = XL9555_DEV_ADDR,
        .scl_speed_hz = XL9555_I2C_SPEED,
    };
    err = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Add device failed");
        i2c_del_master_bus(bus_handle);
        bus_handle = NULL;
        return err;
    }

    // 清空输出寄存器，安全默认状态
    err = write_reg(REG_OUTPUT_PORT_0, 0x00);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear output port 0");
        return err;
    }
    err = write_reg(REG_OUTPUT_PORT_1, 0x00);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear output port 1");
        return err;
    }

    ESP_LOGI(TAG, "XL9555 init success");
    return ESP_OK;
}

esp_err_t xl9555_set_pin_mode(uint8_t port, uint8_t pin, bool is_output)
{
    if (port > 1 || pin > 7) return ESP_ERR_INVALID_ARG;
    uint8_t reg = (port == 0) ? REG_CONFIG_PORT_0 : REG_CONFIG_PORT_1;
    uint8_t bit_mask = (1 << pin);
    return modify_reg(reg, bit_mask, !is_output);
}

esp_err_t xl9555_write_pin(uint8_t port, uint8_t pin, bool level)
{
    if (port > 1 || pin > 7) return ESP_ERR_INVALID_ARG;
    uint8_t reg = (port == 0) ? REG_OUTPUT_PORT_0 : REG_OUTPUT_PORT_1;
    uint8_t bit_mask = (1 << pin);
    return modify_reg(reg, bit_mask, level);
}

esp_err_t xl9555_read_pin(uint8_t port, uint8_t pin, bool *level)
{
    if (port > 1 || pin > 7 || level == NULL) return ESP_ERR_INVALID_ARG;
    uint8_t reg = (port == 0) ? REG_INPUT_PORT_0 : REG_INPUT_PORT_1;
    uint8_t reg_val;
    esp_err_t err = read_reg(reg, &reg_val);
    if (err != ESP_OK) return err;
    *level = (reg_val & (1 << pin)) ? true : false;
    return ESP_OK;
}