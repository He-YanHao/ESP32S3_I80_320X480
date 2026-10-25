#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_init.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lv_adapter.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "gui_guider.h"
#include "gg_utils.h"
#include "custom.h"

// 定义全局 UI 结构体（extern 声明在 gui_guider.h 中）
gg_ui_t guider_ui;


static const char *TAG = "lvgl_init";

static esp_lcd_panel_handle_t s_panel = NULL;
static lv_display_t          *s_disp  = NULL;
static lv_indev_t            *s_indev = NULL;

// 局部刷新缓冲（与旧版 LVGL8 一致的 60 行，从 PSRAM 分配）
#define DRAW_BUF_LINES    60
#define BUF_SIZE_PIXELS   (LCD_H_RES * DRAW_BUF_LINES)
#define BUF_SIZE_BYTES    (BUF_SIZE_PIXELS * 2)   // RGB565

static void *buf1 = NULL;
static void *buf2 = NULL;

/*
 * DMA 颜色传输完成回调（中断上下文）。
 *
 * esp_lcd_panel_draw_bitmap() 只是把传输挂到 DMA 队列就返回，真正的发送是异步的。
 * 必须等这笔传输结束后再通知 LVGL 复用绘制缓冲，否则双缓冲可能在 DMA 还没读完时
 * 就被下一帧覆盖，表现为撕裂/花屏。
 */
static bool lvgl_color_trans_done_cb(esp_lcd_panel_io_handle_t io,
                                     esp_lcd_panel_io_event_data_t *edata,
                                     void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

/*
 * 手动 flush 回调：直接把 LVGL 原生 RGB565 数据交给 esp_lcd_panel_draw_bitmap 发送，
 * 不做任何字节交换。
 *
 * 之所以不用 esp_lv_adapter_register_display()：adapter 对 "OTHER"(I80/SPI) + RGB565
 * 会在 flush 阶段无条件做一次 lv_draw_sw_rgb565_swap（软件字节交换），且没有关闭选项。
 * 而本屏的 16bit I80 总线需要原生 RGB565 字节序（纯色直刷测试已验证），多这一次交换
 * 会导致背景偏绿。因此这里绕开 adapter 的显示注册，手动注册显示 + 自定义 flush 回调。
 */
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1,
                                              area->x2 + 1, area->y2 + 1, px_map);
    if (err != ESP_OK) {
        // 数据没发出去就不会有传输完成回调，这里必须直接放行，避免 LVGL 永久等待
        ESP_LOGE(TAG, "draw_bitmap failed: %s", esp_err_to_name(err));
        lv_display_flush_ready(disp);
    }
}

/*
 * 触摸方向配置。
 * GT1151Q 上报的是面板原始坐标；若点击位置与画面方向不一致，改下面的宏即可：
 *   - TOUCH_SWAP_XY：交换 X/Y（横竖屏方向相反时打开）
 *   - TOUCH_MIRROR_X / TOUCH_MIRROR_Y：镜像对应轴
 * 本屏 bsp_lcd.c 里对 LCD 做了 x 轴镜像（esp_lcd_panel_mirror(true, false)），
 * 如果触摸左右相反，把 TOUCH_MIRROR_X 改成 1。
 */
#define TOUCH_SWAP_XY     0
#define TOUCH_MIRROR_X    0
#define TOUCH_MIRROR_Y    0

static void lvgl_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;

    bsp_touch_point_t pt;
    if (bsp_touch_read(&pt) != ESP_OK || !pt.pressed) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    int32_t x = pt.x;
    int32_t y = pt.y;

#if TOUCH_SWAP_XY
    int32_t tmp = x;
    x = y;
    y = tmp;
#endif
#if TOUCH_MIRROR_X
    x = (TOUCH_SWAP_XY ? LCD_V_RES : LCD_H_RES) - 1 - x;
#endif
#if TOUCH_MIRROR_Y
    y = (TOUCH_SWAP_XY ? LCD_H_RES : LCD_V_RES) - 1 - y;
#endif

    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
}

bool lvgl_lock(void)
{
    return esp_lv_adapter_lock(-1) == ESP_OK;
}

bool lvgl_lock_timeout(uint32_t timeout_ms)
{
    return esp_lv_adapter_lock((int32_t)timeout_ms) == ESP_OK;
}

void lvgl_unlock(void)
{
    esp_lv_adapter_unlock();
}

esp_err_t lvgl_init(void)
{
    // 1. 初始化 LCD 硬件（I80 + ST7796）
    ESP_RETURN_ON_ERROR(bsp_lcd_init(), TAG, "bsp_lcd_init failed");
    s_panel = bsp_lcd_get_panel();
    ESP_RETURN_ON_FALSE(s_panel, ESP_ERR_INVALID_STATE, TAG, "LCD handle null");

    // 2. 初始化触摸硬件
    ESP_RETURN_ON_ERROR(bsp_touch_init(), TAG, "bsp_touch_init failed");

    // 3. 初始化 esp_lv_adapter（负责 lv_init、互斥锁、tick 定时器、LVGL 渲染任务）
    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    cfg.task_core_id = 1;
    ESP_RETURN_ON_ERROR(esp_lv_adapter_init(&cfg), TAG, "adapter init failed");

    // 4. 手动注册显示设备（不使用 esp_lv_adapter_register_display，避免其软件字节交换）
    buf1 = heap_caps_malloc(BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = heap_caps_malloc(BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(buf1 && buf2, ESP_ERR_NO_MEM, TAG, "alloc draw buffers failed");

    s_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    ESP_RETURN_ON_FALSE(s_disp, ESP_ERR_NO_MEM, TAG, "display create failed");
    lv_display_set_flush_cb(s_disp, lvgl_flush_cb);
    lv_display_set_buffers(s_disp, buf1, buf2, BUF_SIZE_BYTES, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 注册 I80 传输完成回调：DMA 真正发完后再让 LVGL 复用绘制缓冲
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = lvgl_color_trans_done_cb,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_register_event_callbacks(bsp_lcd_get_io(), &cbs, s_disp),
                        TAG, "register io callbacks failed");

    // 5. 手动创建 LVGL 指针输入设备并绑定到该显示
    //    bsp_touch 是裸 I2C 驱动，没有 esp_lcd_touch_handle_t，所以不用 adapter 的触摸注册
    s_indev = lv_indev_create();
    ESP_RETURN_ON_FALSE(s_indev, ESP_ERR_NO_MEM, TAG, "indev create failed");
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, lvgl_touch_read_cb);
    lv_indev_set_display(s_indev, s_disp);

    // 6. 启动适配器任务（内部会周期调用 lv_timer_handler）
    ESP_RETURN_ON_ERROR(esp_lv_adapter_start(), TAG, "adapter start failed");

    // GUI Guider 2.x：setup_ui 内部已包含 lay_sys/top/bottom/screen 的创建
    // 以及各自的 gg_event_init_*，无需再调用 events_init。
    lvgl_lock();
    setup_ui(&guider_ui);
    custom_init(&guider_ui);

    // 打一条内存日志：用于评估 LV_MEM_SIZE（当前 256KB）是否过大
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    ESP_LOGI(TAG, "LVGL mem: used %u/%u (%u%%), max_used %u, frag %u%%",
             (unsigned)(mon.total_size - mon.free_size), (unsigned)mon.total_size,
             (unsigned)mon.used_pct, (unsigned)mon.max_used, (unsigned)mon.frag_pct);
    lvgl_unlock();

    ESP_LOGI(TAG, "LVGL init ok (manual display, I80)");
    return ESP_OK;
}
