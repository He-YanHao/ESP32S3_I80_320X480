# lvgl — ESP32-S3 + ST7796 LVGL 9.4 图形项目

基于 ESP-IDF v6.1 的 ESP32-S3 项目，驱动 ST7796（320×480）I80 16bit 并行总线 LCD，
使用 LVGL 9.4.0 做图形界面。

## 硬件

| 模块 | 型号 / 说明 |
|------|------------|
| MCU | ESP32-S3（8MB PSRAM，16MB Flash） |
| 显示屏 | ST7796，320×480，RGB565，I80 16bit 并行总线 |
| 触摸 | GT1151Q（I2C，地址 0x14） |
| IO 扩展 | XL9555（控制 LCD 复位 / 背光等） |

## 目录结构

```
main/                    # 入口 app_main
components/
  bsp_exio/              # XL9555 IO 扩展驱动
  bsp_lcd/               # ST7796 I80 LCD 驱动
  bsp_touch/             # GT1151Q 触摸驱动（裸 I2C）
  lvgl_init/             # LVGL 显示手动注册 + 线程安全锁封装
  lvgl__lvgl/            # LVGL 9.4.0 源码
  guider_ui/             # NXP GUI Guider 生成的界面
managed_components/      # esp_lvgl_adapter、esp_lcd_st7796 等依赖
docs/                    # 文档（含 LVGL 9.4 升级排查记录）
```

## 构建与烧录

```fish
# 配置 ESP-IDF 环境（fish）
. /home/he/.espressif/v6.1/esp-idf/export.fish

# 构建
idf.py build

# 烧录并监视串口
idf.py -p /dev/ttyACM0 flash monitor
```

串口 `/dev/ttyACM0`，波特率 115200。

## 依赖

- ESP-IDF v6.1
- [lvgl/lvgl](https://github.com/lvgl/lvgl) 9.4.0（本地组件 `components/lvgl__lvgl`）
- [espressif/esp_lvgl_adapter](https://github.com/espressif/esp-iot-solution/tree/master/components/display/tools/esp_lvgl_adapter)（仅用于 `lv_init`/互斥锁/tick/渲染任务）
- [espressif/esp_lcd_st7796](https://components.espressif.com/components/espressif/esp_lcd_st7796)

## 注意事项

1. **跨任务操作 LVGL 必须加锁**：LVGL 9 渲染在独立任务里运行，`app_main` 等其它任务
   操作 LVGL 需用 `lvgl_lock()/lvgl_unlock()` 包住。
2. **本屏无需字节交换**：显示设备手动注册，flush 回调不做软件字节交换（详见
   [docs/lvgl94-migration.md](docs/lvgl94-migration.md)）。

## 升级问题记录

从 LVGL 8.3 升级到 9.4 时踩过的两个坑（崩溃、颜色偏绿）及修复，见
[docs/lvgl94-migration.md](docs/lvgl94-migration.md)。
