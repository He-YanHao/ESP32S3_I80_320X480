# CLAUDE.md

本文件为 Claude Code 在此仓库工作时的指引。

## 项目简介

ESP32-S3 驱动 ST7796（320×480，I80 16bit 并行总线）的 LVGL 9.4 图形项目。
含 8MB PSRAM、16MB Flash、GT1151Q 触摸、XL9555 IO 扩展。

## 构建与烧录

必须先 source ESP-IDF 环境（用户 shell 是 fish）：

```fish
. /home/he/.espressif/v6.1/esp-idf/export.fish
```

> 注意：如果是在 bash 里执行（如工具调用），请改用 `export.sh`：
> `. /home/he/.espressif/v6.1/esp-idf/export.sh`

构建、烧录并监视：

```bash
idf.py build
idf.py -p /dev/ttyACM0 flash
idf.py -p /dev/ttyACM0 monitor   # 或 flash monitor 一次完成
```

- 串口：`/dev/ttyACM0`，波特率 115200。
- 只能修改 `/home/he/esp/s3_320x480` 内的内容，不要动外部文件。

## 目录结构

- `main/` — 入口 `app_main`（EXIO → FATFS → LVGL → UI → 主循环）。
- `components/bsp_exio/` — XL9555 IO 扩展（控制 LCD 复位/背光等）。
- `components/bsp_lcd/` — ST7796 I80 LCD 驱动（基于 `esp_lcd_st7796`）。
- `components/bsp_touch/` — GT1151Q 触摸（裸 I2C 驱动）。
- `components/lvgl_init/` — LVGL 显示手动注册 + `lvgl_lock` 线程安全封装。
- `components/lvgl__lvgl/` — LVGL 9.4.0 源码（本地组件）。
- `components/guider_ui/` — NXP GUI Guider 生成的界面（当前未在 main 中使用）。
- `managed_components/` — 依赖（`esp_lvgl_adapter`、`esp_lcd_st7796` 等）。

## 关键约束（务必遵守）

### 1. 跨任务操作 LVGL 必须加锁

LVGL 9 的渲染由 `esp_lvgl_adapter` 的独立任务（核 1）执行。任何在 `app_main`
或自建任务里对 LVGL 的调用（创建控件、改图片源、刷新等）都必须用锁包住：

```c
lvgl_lock();
// ... LVGL 调用 ...
lvgl_unlock();
```

否则会触发 `lv_inv_area` 断言崩溃（`Invalidate area is not allowed during rendering`）。

### 2. 本屏不需要字节交换，且不要用 adapter 自动注册显示

- 显示设备在 `components/lvgl_init/lvgl_init.c` 里**手动注册**（`lv_display_create` +
  自定义 flush 回调），flush 回调里**不做任何软件字节交换**。
- 不要改回 `esp_lv_adapter_register_display()`：它对 I80+RGB565 会自动做
  `lv_draw_sw_rgb565_swap`，导致屏幕偏绿。
- `bsp_lcd.c` 里保留 `ST7796_PANEL_IO_I80_CONFIG` 的 `swap_color_bytes = 1`（在本屏
  16bit 总线上不改变字节序），不要额外改。

详见 [docs/lvgl94-migration.md](docs/lvgl94-migration.md)。

### 3. I80 DMA 大小

`bsp_lcd.c` 里 `MAX_TRANSFER_BYTES = LCD_H_RES * LCD_V_RES * 2`（全帧 300KB）是需要的，
实测正常工作，无需缩小。I80 驱动的 DMA 描述符会自动按 4095 字节分段。

## 常用约定

- 代码注释与文档用中文。
- 不要删除 `main.c` 主循环里的 `ESP_LOGI(TAG, "running...")`，用户用它判断系统存活。
