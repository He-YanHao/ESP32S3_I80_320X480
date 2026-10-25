# LVGL 8.3 → 9.4 升级问题排查与修复

本文记录本项目从 LVGL 8.3 升级到 LVGL 9.4 后遇到的两个问题及其根因，供后续维护参考。

## 背景

- 硬件：ESP32-S3（8MB PSRAM / 16MB Flash）+ ST7796（320×480，I80 16bit 并行总线）+ GT1151Q 触摸 + XL9555 IO 扩展。
- 升级前：LVGL 8.3，直接手写显示驱动注册，`lv_timer_handler()` 在 `app_main` 主循环里调用。
- 升级后：LVGL 9.4 + Espressif `esp_lvgl_adapter`（负责 `lv_init`、互斥锁、tick 定时器、以及一个独立的 LVGL 渲染任务）。

升级后出现两类问题：**启动即崩溃** 和 **颜色偏绿**。

---

## 问题一：启动崩溃（线程安全）

### 现象

```
lv_inv_area: Asserted at expression: !disp->rendering_in_progress
    (Invalidate area is not allowed during rendering.) lv_refr.c:282
```

随后触发 task watchdog，`app_main` 卡在 `lv_label_create` 里。

### 根因

`esp_lv_adapter` 会创建一个**独立的 LVGL 渲染任务**（默认绑核 1），在任务里周期调用
`lv_timer_handler()`。而 `app_main` 里直接调用 `lv_label_create()` 等 LVGL API，
此时渲染任务可能正在 `lv_timer_handler()` 内部渲染，两者并发访问 LVGL，触发断言。

这是 LVGL 9 的硬性约束：**渲染进行中不允许 invalidate 区域**。LVGL 8 时代因为
`lv_timer_handler()` 由应用自己在主循环调用，天然单线程，不存在这个问题。

### 修复

所有**非 LVGL 渲染任务**里对 LVGL 对象的操作，必须用锁包住：

```c
lvgl_lock();
lv_obj_t *label = lv_label_create(lv_screen_active());
lv_label_set_text(label, "Hello LVGL 9.4");
lv_obj_center(label);
lvgl_unlock();
```

`lvgl_lock()/lvgl_unlock()` 封装了 adapter 的 `esp_lv_adapter_lock()/esp_lv_adapter_unlock()`，
见 [components/lvgl_init/lvgl_init.c](../components/lvgl_init/lvgl_init.c)。

> 教训：以后在 `app_main` 或任何自建任务里操作 LVGL（创建控件、改图片源、刷新等），
> 都必须先 `lvgl_lock()`，操作完再 `lvgl_unlock()`。

---

## 问题二：屏幕偏绿、文字模糊（字节序）

### 现象

背景本应是接近白色的浅色（LVGL 默认主题，实测渲染值为 `0xF7BE`），
但屏幕显示为**淡绿色**，文字模糊。

### 根因（双重字节交换）

`0xF7BE` 与 `0xBEF7` 恰好互为 RGB565 两字节交换的结果：
`0xBEF7 = (189, 222, 189)` 即淡绿色。也就是说屏幕收到的是**字节被交换过一次**的数据。

问题出在 `esp_lvgl_adapter` 的 flush 阶段：它对 I80（`PANEL_IF_OTHER`）+ RGB565
会**自动做一次软件字节交换**（`lv_draw_sw_rgb565_swap`，见 adapter 的
`display_bridge_v9_flush_default`）。这一步是本屏**不需要**的——本屏的 16bit I80 总线
上数据直接按 LVGL 原生 RGB565 发送就是正确颜色（纯色测试已验证）。

因此 adapter 的软件交换把 `0xF7BE` 变成了 `0xBEF7`，屏幕自然偏绿。

### 修复

**不使用** `esp_lv_adapter_register_display()` 自动注册显示（它会接管 flush 并做软件交换），
改为**手动注册 LVGL 显示**，flush 回调里不做任何字节交换，直接发原生数据：

```c
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_display_flush_ready(disp);
}
```

完整实现见 [components/lvgl_init/lvgl_init.c](../components/lvgl_init/lvgl_init.c)。

### 说明

- LVGL 8 时代的 `LV_COLOR_16_SWAP` 配置项在 LVGL 9 里已移除，字节序改由颜色格式
  （`LV_COLOR_FORMAT_RGB565` / `LV_COLOR_FORMAT_RGB565_SWAPPED`）表达。
- 本屏经纯色测试确认：**原生 RGB565 直接发送即正确**，无需任何字节交换。
- `bsp_lcd.c` 里 `ST7796_PANEL_IO_I80_CONFIG` 宏自带的 `swap_color_bytes = 1`
  （硬件 `lcd_8bits_order` 寄存器）在本屏的 16bit 总线上实际不改变字节序，保留即可。

### 为什么最终选择手动注册（而不是用 adapter 的其它能力）

排查过程中确认：`esp_lvgl_adapter` 对 I80+RGB565 的软件交换是**写死的，没有关闭开关**：

- `Kconfig` 无字节序选项；`esp_lv_adapter_display_profile_t` / `_config_t` 无字节序字段；
- 官方文档与 `MIGRATION_GUIDE` 未提字节序配置（旧版 `lvgl_port` 曾有 `swap bytes` 标志，新版已移除）。

尝试过的备选方案及放弃原因：

1. **自定义 `draw_bitmap` 回调**（`esp_lv_adapter_set_draw_bitmap_callbacks`）：回调在软件交换**之后**被调用，拿到的是已交换数据，只能在回调里再交换一次还原，等于"交换→交换回来"，冗余且浪费。
2. **直接写硬件寄存器 `LCD_CAM.lcd_user.lcd_byte_order`**（ESP32-S3 的 16bit 字节反转，仅 16bit 模式有效）：能让 adapter 软件交换与硬件反转抵消、零冗余，但属于直接操作硬件寄存器、绕过驱动抽象，风险偏高，故放弃。

最终采用**手动注册显示**（绕开 adapter 的 flush），最直观、无冗余、不碰硬件寄存器。

---

## 最终架构

- `esp_lv_adapter` 仅用于：`lv_init()`、互斥锁、tick 定时器、LVGL 渲染任务。
- 显示设备**手动注册**（`lv_display_create` + 自定义 flush 回调 + 双缓冲）。
- 触摸暂未通过 adapter 注册（`bsp_touch_init` 是裸 I2C 驱动，未封装成 `esp_lcd_touch_handle_t`）。

## 关键结论

1. LVGL 9 渲染是独立任务，跨任务操作 LVGL 必须加锁。
2. 本屏不需要字节交换；`esp_lvgl_adapter` 对 I80+RGB565 的自动软件交换是多余的，需手动注册显示绕开。
3. 排查颜色问题时，先用**绕过 LVGL 的纯色直刷**验证面板硬件/驱动配置，再排查 LVGL 链路，能快速定位问题层。
