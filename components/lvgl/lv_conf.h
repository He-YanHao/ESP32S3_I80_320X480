/**
 * @file lv_conf.h
 * Configuration file for v8.4.0
 */

/*
 * Copy this file as `lv_conf.h`
 * 1. simply next to the `lvgl` folder
 * 2. or any other places and
 *    - define `LV_CONF_INCLUDE_SIMPLE`
 *    - add the path as include path
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
// RGB565格式的颜色数据在内存中的存储顺序，0表示正常顺序，1表示交换顺序，根据显示屏的接口类型选择正确的值。
// 对于SPI接口的显示屏，通常需要交换字节顺序，因此设置为1。
#define LV_COLOR_16_SWAP 0

/*Enable features to draw on transparent background.
 *It's required if opa, and transform_* style properties are used.
 *Can be also used if the UI is above another layer, e.g. an OSD menu or video player.*/
// 启用在透明背景上绘制的功能。这对于使用opa（不透明度）和transform_*（变换）样式属性是必需的。
// 也可以在UI位于另一个层之上时使用，例如OSD菜单或视频播放器。
// 比较消耗性能，除非需要，否则建议保持为0。
#define LV_COLOR_SCREEN_TRANSP 0

/* Adjust color mix functions rounding. GPUs might calculate color mix (blending) differently.
 * 0: round down, 64: round up from x.75, 128: round up from half, 192: round up from x.25, 254: round up */
#define LV_COLOR_MIX_ROUND_OFS 0

/*Images pixels with this color will not be drawn if they are chroma keyed)*/
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)         /*pure green*/

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()`*/
// 0：使用 LVGL 内置的静态内存池
// 1：使用自定义的 malloc/free 函数，通常是为了使用动态内存分配（如 heap）
#define LV_MEM_CUSTOM 0   /* 改为 0，启用内置内存池，但通过 LV_MEM_POOL_ALLOC 动态分配池 */
#if LV_MEM_CUSTOM == 0
    /* 内置内存池大小（当 LV_MEM_ADR != 0 时静态分配，此处动态分配则会被忽略） */
    #define LV_MEM_SIZE (4U * 1024U * 1024U)   /* [bytes] */
    /* 0 表示不使用固定地址，而是通过 LV_MEM_POOL_ALLOC 动态分配 */
    #define LV_MEM_ADR 0
    #if LV_MEM_ADR == 0
        /* 先取消可能的默认定义，再重新定义为使用 PSRAM 的分配器 */
        #undef LV_MEM_POOL_INCLUDE
        #define LV_MEM_POOL_INCLUDE "esp_heap_caps.h"
        #undef LV_MEM_POOL_ALLOC
        #define LV_MEM_POOL_ALLOC(size) heap_caps_malloc(size, MALLOC_CAP_SPIRAM)
    #endif
#else   /* LV_MEM_CUSTOM == 1 分支（如果你需要使用完全自定义的分配器，可参考下方） */
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif

/*Number of the intermediate memory buffer used during rendering and other internal processing mechanisms.
 *You will see an error log message if there wasn't enough buffers. */
#define LV_MEM_BUF_MAX_NUM 128

/*Use the standard `memcpy` and `memset` instead of LVGL's own functions. (Might or might not be faster).*/
#define LV_MEMCPY_MEMSET_STD 0

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period. LVG will redraw changed areas with this period time*/
// 显示刷新周期，单位为毫秒。LVGL 将以此周期重绘已更改的区域。较短的周期可以提供更流畅的动画，但会增加 CPU 负载和功耗。
// 根据应用需求调整此值。
#define LV_DISP_DEF_REFR_PERIOD   30      /*[ms]*/

// 输入设备读取周期，单位为毫秒。LVGL 将以此周期读取输入设备的数据。较短的周期可以提供更快的响应，但会增加 CPU 负载。
// 根据输入设备的类型和应用需求调整此值。
/*Input device read period in milliseconds*/
#define LV_INDEV_DEF_READ_PERIOD  30     /*[ms]*/

/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
// 启用自定义的tick源，直接提供一个表达式来获取当前的系统时间（以毫秒为单位）。
// 这样就不需要在主循环中调用lv_tick_inc()来更新tick了。
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    // 这里直接给出了ESP32的案例代码
    #define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR ((esp_timer_get_time() / 1000LL))
#endif   /*LV_TICK_CUSTOM*/

/*Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 *(Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI_DEF 130     /*[px/inch]*/

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*-------------
 * Drawing
 *-----------*/

/*Enable complex draw engine.
 *Required to draw shadow, gradient, rounded corners, circles, arc, skew lines, image transformations or any masks*/
// 启用复杂绘制引擎，支持阴影、渐变、圆角、圆形、弧线、倾斜线、图像变换和遮罩等高级绘制功能。
#define LV_DRAW_COMPLEX 1
#if LV_DRAW_COMPLEX != 0

    /*Allow buffering some shadow calculation.
    *LV_SHADOW_CACHE_SIZE is the max. shadow size to buffer, where shadow size is `shadow_width + radius`
    *Caching has LV_SHADOW_CACHE_SIZE^2 RAM cost*/
   // 允许缓存一些阴影计算结果，以提高性能。LV_SHADOW_CACHE_SIZE 定义了要缓存的最大阴影大小（shadow_width + radius）。
   // 缓存会占用 LV_SHADOW_CACHE_SIZE^2 的 RAM。
    #define LV_SHADOW_CACHE_SIZE 0

    /* Set number of maximally cached circle data.
    * The circumference of 1/4 circle are saved for anti-aliasing
    * radius * 4 bytes are used per circle (the most often used radiuses are saved)
    * 0: to disable caching */
    // 设置最大缓存的圆数据数量。为了抗锯齿，1/4 圆的周长被保存。每个圆使用 radius * 4 字节（最常用的半径被保存）。
    // 0 表示禁用缓存。
    #define LV_CIRCLE_CACHE_SIZE 4
#endif /*LV_DRAW_COMPLEX*/

/**
 * "Simple layers" are used when a widget has `style_opa < 255` to buffer the widget into a layer
 * and blend it as an image with the given opacity.
 * Note that `bg_opa`, `text_opa` etc don't require buffering into layer)
 * The widget can be buffered in smaller chunks to avoid using large buffers.
 *
 * - LV_LAYER_SIMPLE_BUF_SIZE: [bytes] the optimal target buffer size. LVGL will try to allocate it
 * - LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE: [bytes]  used if `LV_LAYER_SIMPLE_BUF_SIZE` couldn't be allocated.
 *
 * Both buffer sizes are in bytes.
 * "Transformed layers" (where transform_angle/zoom properties are used) use larger buffers
 * and can't be drawn in chunks. So these settings affects only widgets with opacity.
 */
#define LV_LAYER_SIMPLE_BUF_SIZE          (24 * 1024)
#define LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE (3 * 1024)

/*Default image cache size. Image caching keeps the images opened.
 *If only the built-in image formats are used there is no real advantage of caching. (I.e. if no new image decoder is added)
 *With complex image decoders (e.g. PNG or JPG) caching can save the continuous open/decode of images.
 *However the opened images might consume additional RAM.
 *0: to disable caching*/
// 设置图片解码缓存的数量（即可缓存的已打开图片张数）。
// 如果只使用内置的图片格式，缓存没有实际优势（即没有添加新的图片解码器）。
// 对于复杂的图片解码器（如 PNG 或 JPG），缓存可以避免连续打开/解码图片。
// 然而，已打开的图片可能会占用额外的 RAM。
#define LV_IMG_CACHE_DEF_SIZE 0

/*Number of stops allowed per gradient. Increase this to allow more stops.
 *This adds (sizeof(lv_color_t) + 1) bytes per additional stop*/
// 若要更复杂的多彩渐变背景，可酌情增大到所需数量。
#define LV_GRADIENT_MAX_STOPS 2

/*Default gradient buffer size.
 *When LVGL calculates the gradient "maps" it can save them into a cache to avoid calculating them again.
 *LV_GRAD_CACHE_DEF_SIZE sets the size of this cache in bytes.
 *If the cache is too small the map will be allocated only while it's required for the drawing.
 *0 mean no caching.*/
// 默认渐变缓存大小。当 LVGL 计算渐变“映射”时，可以将它们保存到缓存中以避免再次计算。LV_GRAD_CACHE_DEF_SIZE 设置此缓存的大小（以字节为单位）。
// 如果缓存太小，映射将仅在绘制时分配。0 表示不缓存。
#define LV_GRAD_CACHE_DEF_SIZE 0

/*Allow dithering the gradients (to achieve visual smooth color gradients on limited color depth display)
 *LV_DITHER_GRADIENT implies allocating one or two more lines of the object's rendering surface
 *The increase in memory consumption is (32 bits * object width) plus 24 bits * object width if using error diffusion */
// 开启后可以在有限色深的显示屏上实现视觉上更平滑的颜色渐变，但会增加内存消耗，具体增加量取决于对象的宽度和使用的抖动算法。
#define LV_DITHER_GRADIENT 0
#if LV_DITHER_GRADIENT
    /*Add support for error diffusion dithering.
     *Error diffusion dithering gets a much better visual result, but implies more CPU consumption and memory when drawing.
     *The increase in memory consumption is (24 bits * object's width)*/
    // 错误扩散抖动可以获得更好的视觉效果，但会增加 CPU 消耗和绘制时的内存使用。具体增加量取决于对象的宽度。
    #define LV_DITHER_ERROR_DIFFUSION 0
#endif

/*Maximum buffer size to allocate for rotation.
 *Only used if software rotation is enabled in the display driver.*/
// 设置软件旋转所需的最大缓冲区大小。仅当显示驱动程序启用软件旋转时使用。
#define LV_DISP_ROT_MAX_BUF (10*1024)

/*-------------
 * GPU
 * ESP32不属于GPU加速的范围，以下选项保持默认即可
 *-----------*/

/*Use Arm's 2D acceleration library Arm-2D */
#define LV_USE_GPU_ARM2D 0

/*Use STM32's DMA2D (aka Chrom Art) GPU*/
#define LV_USE_GPU_STM32_DMA2D 0
#if LV_USE_GPU_STM32_DMA2D
    /*Must be defined to include path of CMSIS header of target processor
    e.g. "stm32f7xx.h" or "stm32f4xx.h"*/
    #define LV_GPU_DMA2D_CMSIS_INCLUDE
#endif

/*Enable RA6M3 G2D GPU*/
#define LV_USE_GPU_RA6M3_G2D 0
#if LV_USE_GPU_RA6M3_G2D
    /*include path of target processor
    e.g. "hal_data.h"*/
    #define LV_GPU_RA6M3_G2D_INCLUDE "hal_data.h"
#endif

/*Use SWM341's DMA2D GPU*/
#define LV_USE_GPU_SWM341_DMA2D 0
#if LV_USE_GPU_SWM341_DMA2D
    #define LV_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#endif

/*Use NXP's PXP GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_PXP 0
#if LV_USE_GPU_NXP_PXP
    /*1: Add default bare metal and FreeRTOS interrupt handling routines for PXP (lv_gpu_nxp_pxp_osa.c)
    *   and call lv_gpu_nxp_pxp_init() automatically during lv_init(). Note that symbol SDK_OS_FREE_RTOS
    *   has to be defined in order to use FreeRTOS OSA, otherwise bare-metal implementation is selected.
    *0: lv_gpu_nxp_pxp_init() has to be called manually before lv_init()
    */
    #define LV_USE_GPU_NXP_PXP_AUTO_INIT 0
#endif

/*Use NXP's VG-Lite GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_VG_LITE 0

/*Use SDL renderer API*/
// 使用 SDL 渲染 API 进行 GPU 加速，适用于在 PC 上使用 SDL 模拟显示和输入设备的开发环境。
#define LV_USE_GPU_SDL 0
#if LV_USE_GPU_SDL
    #define LV_GPU_SDL_INCLUDE_PATH <SDL2/SDL.h>
    /*Texture cache size, 8MB by default*/
    #define LV_GPU_SDL_LRU_SIZE (1024 * 1024 * 8)
    /*Custom blend mode for mask drawing, disable if you need to link with older SDL2 lib*/
    #define LV_GPU_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
#endif

/*-------------
 * Logging
 *-----------*/

/*Enable the log module*/
// 日志输出系统，LV_USE_LOG 设置为 1 时启用日志模块，可以根据需要调整日志级别和输出方式。
#define LV_USE_LOG 1
#if LV_USE_LOG

    /*How important log should be added:
    *LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
    *LV_LOG_LEVEL_INFO        Log important events
    *LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
    *LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
    *LV_LOG_LEVEL_USER        Only logs added by the user
    *LV_LOG_LEVEL_NONE        Do not log anything*/
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /*1: Print the log with 'printf';
    *0: User need to register a callback with `lv_log_register_print_cb()`*/
    #define LV_LOG_PRINTF 1

    /*Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs*/
    #define LV_LOG_TRACE_MEM        1
    #define LV_LOG_TRACE_TIMER      1
    #define LV_LOG_TRACE_INDEV      1
    #define LV_LOG_TRACE_DISP_REFR  1
    #define LV_LOG_TRACE_EVENT      1
    #define LV_LOG_TRACE_OBJ_CREATE 1
    #define LV_LOG_TRACE_LAYOUT     1
    #define LV_LOG_TRACE_ANIM       1

#endif  /*LV_USE_LOG*/

/*-------------
 * Asserts
 *-----------*/

/*Enable asserts if an operation is failed or an invalid data is found.
 *If LV_USE_LOG is enabled an error message will be printed on failure*/
#define LV_USE_ASSERT_NULL          1   /*Check if the parameter is NULL. (Very fast, recommended)*/
#define LV_USE_ASSERT_MALLOC        1   /*Checks is the memory is successfully allocated or no. (Very fast, recommended)*/
#define LV_USE_ASSERT_STYLE         0   /*Check if the styles are properly initialized. (Very fast, recommended)*/
#define LV_USE_ASSERT_MEM_INTEGRITY 0   /*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#define LV_USE_ASSERT_OBJ           0   /*Check the object's type and existence (e.g. not deleted). (Slow)*/

/*Add a custom handler when assert happens e.g. to restart the MCU*/
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);   /*Halt by default*/

/*-------------
 * Others
 *-----------*/

/*1: Show CPU usage and FPS count*/
// 启用性能监视器，显示 CPU 使用率和 FPS 计数。根据需要启用此功能以监控性能指标。
#define LV_USE_PERF_MONITOR 1
#if LV_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

/*1: Show the used memory and the memory fragmentation
 * Requires LV_MEM_CUSTOM = 0*/
// 启用内存监视器，显示已使用的内存和内存碎片情况。仅当 LV_MEM_CUSTOM 设置为 0（使用 LVGL 内置的内存管理）时有效。
#define LV_USE_MEM_MONITOR 1
#if LV_USE_MEM_MONITOR
    #define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
#endif

/*1: Draw random colored rectangles over the redrawn areas*/
// 刷新区域调试。开启后，每次屏幕刷新时，所有被重绘的脏区域会被随机颜色的半透明矩形覆盖。
// 这对于调试哪些区域被重绘以及重绘的频率非常有用，但会显著影响性能和视觉效果，因此仅在调试阶段使用。
#define LV_USE_REFR_DEBUG 0

/*Change the built in (v)snprintf functions*/
#define LV_SPRINTF_CUSTOM 0
#if LV_SPRINTF_CUSTOM
    #define LV_SPRINTF_INCLUDE <stdio.h>
    #define lv_snprintf  snprintf
    #define lv_vsnprintf vsnprintf
#else   /*LV_SPRINTF_CUSTOM*/
    #define LV_SPRINTF_USE_FLOAT 0
#endif  /*LV_SPRINTF_CUSTOM*/

// 允许在 LVGL 对象（lv_obj_t）上挂载用户自定义数据指针。
// 启用后，每个 LVGL 对象将包含一个 void* 类型的 user_data 字段，用户可以使用它来存储与该对象相关的任意数据。
// 这对于在事件回调中访问对象特定的数据非常有用。
#define LV_USE_USER_DATA 1

/*Garbage Collector settings
 *Used if lvgl is bound to higher level language and the memory is managed by that language*/
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
    #define LV_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#endif /*LV_ENABLE_GC*/

/*=====================
 *  COMPILER SETTINGS
 *====================*/
// 以下是与编译器相关的设置，主要用于定义一些特定于平台或编译器的属性和功能，以确保 LVGL 在不同环境下的兼容性和性能优化。
// 这些设置通常不需要修改，除非你有特定的需求或正在使用特殊的硬件平台。
// 仅仅在极限优化或特定平台适配时才需要调整这些选项。

/*For big endian systems set to 1*/
#define LV_BIG_ENDIAN_SYSTEM 0

/*Define a custom attribute to `lv_tick_inc` function*/
#define LV_ATTRIBUTE_TICK_INC

/*Define a custom attribute to `lv_timer_handler` function*/
#define LV_ATTRIBUTE_TIMER_HANDLER

/*Define a custom attribute to `lv_disp_flush_ready` function*/
#define LV_ATTRIBUTE_FLUSH_READY

/*Required alignment size for buffers*/
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary by default).
 * E.g. __attribute__((aligned(4)))*/
#define LV_ATTRIBUTE_MEM_ALIGN

/*Attribute to mark large constant arrays for example font's bitmaps*/
#define LV_ATTRIBUTE_LARGE_CONST

/*Compiler prefix for a big array declaration in RAM*/
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*Place performance critical functions into a faster memory (e.g RAM)*/
#define LV_ATTRIBUTE_FAST_MEM

/*Prefix variables that are used in GPU accelerated operations, often these need to be placed in RAM sections that are DMA accessible*/
#define LV_ATTRIBUTE_DMA

/*Export integer constant to binding. This macro is used with constants in the form of LV_<CONST> that
 *should also appear on LVGL binding API such as Micropython.*/
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*The default value just prevents GCC warning*/

/*Extend the default -32k..32k coordinate range to -4M..4M by using int32_t for coordinates instead of int16_t*/
#define LV_USE_LARGE_COORD 0

/*==================
 *   FONT USAGE
 *===================*/

/*Montserrat fonts with ASCII range and some symbols using bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

/*Demonstrate special features*/
// 以下是一些特殊功能的字体示例，启用后可以展示这些功能，但会增加内存使用。根据需要启用。
// 启用后会增加内存使用，除非需要，否则建议保持为0。
// 12像素的蒙特塞拉特字体，启用子像素渲染功能，可以在支持子像素渲染的显示屏上获得更清晰的文本显示效果。
#define LV_FONT_MONTSERRAT_12_SUBPX      0
// 28像素的蒙特塞拉特字体，启用压缩功能，使用每像素3位（bpp = 3）的压缩格式，可以在保持较小内存占用的同时提供较好的视觉质量。
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0  /*bpp = 3*/
// 16像素的DejaVu字体，启用阿拉伯语、希伯来语和波斯语字符处理功能，可以正确显示这些语言的字母及其各种形式。
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /*Hebrew, Arabic, Persian letters and all their forms*/
// 16像素的宋体字体，启用CJK字符支持，包含1000个最常用的中日韩汉字部首，可以在需要显示这些字符的应用中使用。
#define LV_FONT_SIMSUN_16_CJK            0  /*1000 most common CJK radicals*/

/*Pixel perfect monospace fonts*/
// 以下是一些像素完美的等宽字体示例，启用后可以获得更清晰的文本显示效果，但会增加内存使用。根据需要启用。
// 8像素的Uniscii字体，启用后可以使用这个像素完美的等宽字体，适合在需要显示ASCII字符的应用中使用。
#define LV_FONT_UNSCII_8  0
// 16像素的Uniscii字体，启用后可以使用这个像素完美的等宽字体，适合在需要显示ASCII字符的应用中使用。
#define LV_FONT_UNSCII_16 0

/*Optionally declare custom fonts here.
 *You can use these fonts as default font too and they will be available globally.
 *E.g. #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)*/
// 可选地在此处声明自定义字体。启用后，可以将这些字体作为默认字体使用，并且它们将在全局范围内可用。
#define LV_FONT_CUSTOM_DECLARE

/*Always set a default font*/
// 始终设置一个默认字体，启用后将使用指定的默认字体进行文本显示。根据需要选择合适的默认字体。
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*Enable handling large font and/or fonts with a lot of characters.
 *The limit depends on the font size, font face and bpp.
 *Compiler error will be triggered if a font needs it.*/
// 某些超大字体或超大字符集（如数万汉字），其字符描述符大小会超过默认的类型限制，需要启用此宏才能存储。
#define LV_FONT_FMT_TXT_LARGE 1

/*Enables/disables support for compressed fonts.*/
// 启用/禁用对压缩字体的支持。压缩字体使用更高效的存储格式，可以在保持较小内存占用的同时提供较好的视觉质量，但会增加解码的 CPU 负载。
#define LV_USE_FONT_COMPRESSED 0

/*Enable subpixel rendering*/
// 启用子像素渲染功能，可以在支持子像素渲染的显示屏上获得更清晰的文本显示效果，但会增加内存使用。根据需要启用。
#define LV_USE_FONT_SUBPX 0
#if LV_USE_FONT_SUBPX
    /*Set the pixel order of the display. Physical order of RGB channels. Doesn't matter with "normal" fonts.*/
    #define LV_FONT_SUBPX_BGR 0  /*0: RGB; 1:BGR order*/
#endif

/*Enable drawing placeholders when glyph dsc is not found*/
// 启用当找不到字形描述符时绘制占位符的功能。
// 当启用后，如果某个字符的字形描述符未找到，LVGL 将绘制一个占位符（通常是一个方框）来表示该字符，而不是完全不显示。
// 这对于调试和处理缺失字体资源非常有用。
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  TEXT SETTINGS
 *=================*/

/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
// 选择字符串的字符编码。你的 IDE 或编辑器应该使用相同的字符编码。
// 常见选项包括 UTF-8 和 ASCII，根据你的项目需求选择合适的编码。
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*Can break (wrap) texts on these chars*/
// 定义可以在这些字符处断行（换行）。当文本需要换行时，LVGL 将尝试在这些指定的字符处进行断行，以提高文本的可读性和布局效果。
#define LV_TXT_BREAK_CHARS " ,.;:-_"

/*If a word is at least this long, will break wherever "prettiest"
 *To disable, set to a value <= 0*/
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/*Minimum number of characters in a long word to put on a line before a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/*Minimum number of characters in a long word to put on a line after a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*The control character to use for signalling text recoloring.*/
#define LV_TXT_COLOR_CMD "#"

/*Support bidirectional texts. Allows mixing Left-to-Right and Right-to-Left texts.
 *The direction will be processed according to the Unicode Bidirectional Algorithm:
 *https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
// 启用双向文本支持，允许混合使用从左到右（LTR）和从右到左（RTL）的文本。文本的方向将根据 Unicode 双向算法进行处理。
// 启用后，可以通过 LV_BIDI_BASE_DIR_DEF 定义默认的文本方向，支持自动检测文本的基本方向（LTR 或 RTL）。
// 除非你需要显示阿拉伯文、希伯来文等 RTL 语言，否则不必开启。
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /*Set the default direction. Supported values:
    *`LV_BASE_DIR_LTR` Left-to-Right
    *`LV_BASE_DIR_RTL` Right-to-Left
    *`LV_BASE_DIR_AUTO` detect texts base direction*/
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/*Enable Arabic/Persian processing
 *In these languages characters should be replaced with an other form based on their position in the text*/
// UI 如果不包含阿拉伯语/波斯语等，保持关闭是正确的。
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 *  WIDGET USAGE
 *================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/
// 以下是 LVGL 中各种内置组件（widgets）的使用配置选项。启用后，这些组件将可用于你的应用程序中，但会增加内存使用。根据需要启用所需的组件。

// 似乎是 GUI-Guider-1.10.1-GA 的自定义宏
// 用于启用视频播放组件
#define LV_USE_TEXTPROGRESS  0
// 推测是文本进度条
#define LV_USE_VIDEO         0

// 启用弧形组件，允许在界面中使用弧形图形元素，如进度环等。
#define LV_USE_ARC        1

// 启用自动布局组件，提供自动排列子对象的功能，支持水平、垂直和网格布局。
#define LV_USE_BAR        0

// 启用按钮组件，允许在界面中使用可点击的按钮元素，支持文本、图标和事件处理。
#define LV_USE_BTN        1

// 启用按钮矩阵组件，提供一个矩阵布局的按钮集合，适用于需要多个相关按钮的场景，如数字键盘等。
#define LV_USE_BTNMATRIX  1

// 启用画布组件，允许在界面中使用一个可绘制的区域，可以进行像素级的绘制操作，适用于自定义图形和动画。
#define LV_USE_CANVAS     0

// 启用复选框组件，提供一个可切换的选项框，适用于需要多选功能的场景。
#define LV_USE_CHECKBOX   0

// 启用下拉菜单组件，提供一个可展开的选项列表，适用于需要从多个选项中选择一个的场景。
#define LV_USE_DROPDOWN   0   /*Requires: lv_label*/

// 启用图像组件，允许在界面中显示图像元素，支持多种图像格式和变换功能。
#define LV_USE_IMG        1   /*Requires: lv_label*/

// 启用标签组件，提供一个用于显示文本的基本组件，支持多行文本、文本选择和长文本提示等功能。
#define LV_USE_LABEL      1
#if LV_USE_LABEL
    // 启用标签文本选择功能，允许用户选择标签中的文本内容，适用于需要复制或交互的文本显示场景。
    #define LV_LABEL_TEXT_SELECTION 0 /*Enable selecting text of the label*/
    // 启用长文本提示功能，在标签文本过长无法完全显示时，提供额外的信息以加速绘制过程，适用于需要显示大量文本的场景。
    #define LV_LABEL_LONG_TXT_HINT  0 /*Store some extra info in labels to speed up drawing of very long texts*/
#endif

// 启用线条组件，允许在界面中使用线条元素，支持多段线、箭头和线宽等功能。
#define LV_USE_LINE       0

// 启用滚轮组件，提供一个可滚动选择的选项列表，适用于需要在有限空间内显示多个选项的场景，如时间选择器等。
#define LV_USE_ROLLER     0   /*Requires: lv_label*/
#if LV_USE_ROLLER
    // 在滚轮可见区域的上方和下方，会各准备 7 个“虚拟页面”供循环拖动。
    // 数值越大，循环动画越丝滑，越不容易出现露底（滚到空白处），但这会稍微增加重绘的计算量。
    #define LV_ROLLER_INF_PAGES 7 /*Number of extra "pages" when the roller is infinite*/
#endif

// 启用滑块组件，提供一个可拖动的滑块元素，适用于需要调整数值或进度的场景，如音量控制等。
#define LV_USE_SLIDER     0   /*Requires: lv_bar*/

// 启用开关组件，提供一个可切换的开关元素，适用于需要二选一功能的场景，如设置开关等。
#define LV_USE_SWITCH     0

// 启用文本区域组件，提供一个多行文本输入区域，支持文本编辑、光标控制和密码输入等功能，适用于需要用户输入大量文本的场景。
#define LV_USE_TEXTAREA   1   /*Requires: lv_label*/
#if LV_USE_TEXTAREA != 0
    // 启用密码显示功能，当文本区域用于密码输入时，可以暂时显示输入的字符以帮助用户确认输入内容，适用于需要输入密码的场景。
    // 设置密码模式下，单个字符在显示后多久被自动遮盖。
    // 数值越大，用户看到输入的字符的时间越长，但这可能会增加安全风险。根据实际需求调整此值。
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500    /*ms*/
#endif

// 启用表格组件，提供一个用于显示和编辑表格数据的组件，支持单元格合并、排序和样式定制等功能，适用于需要展示结构化数据的场景。
#define LV_USE_TABLE      0

/*==================
 * EXTRA COMPONENTS
 *==================*/

/*-----------
 * Widgets
 *----------*/
// 以下是一些额外的组件选项，这些组件提供了更多的功能和交互方式，但可能会增加内存使用。根据需要启用所需的组件。

// 启用动画图像组件，允许在界面中使用支持动画的图像元素，如 GIF 等，适用于需要展示动态内容的场景。
#define LV_USE_ANIMIMG    0

// 启用日历组件，提供一个用于显示和选择日期的组件，支持月视图、年视图和日期范围选择等功能，适用于需要日期输入或显示的场景。
#define LV_USE_CALENDAR   0
#if LV_USE_CALENDAR
    // 定义一周的起始日。设置为 0 时，周一作为一周的第一天；设置为 1 时，周日作为一周的第一天。根据你的地区习惯选择合适的设置。
    #define LV_CALENDAR_WEEK_STARTS_MONDAY 0
    #if LV_CALENDAR_WEEK_STARTS_MONDAY
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
    #else
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"日", "一", "二", "三", "四", "五", "六"}
    #endif

    // 定义月份名称的默认值。启用后，日历组件将使用这些名称来显示月份。根据需要修改为其他语言或格式。
    #define LV_CALENDAR_DEFAULT_MONTH_NAMES {"一月", "二月", "三月", "四月", "五月", "六月", "七月", "八月", "九月", "十月", "十一月", "十二月"}
    // 启用日历头部的箭头按钮，允许用户通过点击箭头来切换月份或年份，适用于需要快速导航日期的场景。
    #define LV_USE_CALENDAR_HEADER_ARROW    0
    // 启用日历头部的下拉菜单，允许用户通过下拉选择来切换月份或年份，适用于需要快速导航日期的场景。
    #define LV_USE_CALENDAR_HEADER_DROPDOWN 1
#endif  /*LV_USE_CALENDAR*/

// 启用图表组件，提供一个用于显示数据图表的组件，支持折线图、柱状图和饼图等多种图表类型，适用于需要可视化数据的场景。
#define LV_USE_CHART      0

// 启用颜色选择器组件，提供一个用于选择颜色的组件，支持多种颜色模式和预设颜色选项，适用于需要用户选择颜色的场景。
#define LV_USE_COLORWHEEL 0

// 启用图像按钮组件，提供一个结合了图像和按钮功能的组件，允许在按钮上显示图像，适用于需要图文结合的交互元素的场景。
#define LV_USE_IMGBTN     0

// 启用键盘组件，提供一个用于输入文本的虚拟键盘组件，支持多种布局和输入模式，适用于需要在触摸屏上输入文本的场景。
#define LV_USE_KEYBOARD   1

// 启用LED组件，提供一个用于显示状态的LED指示灯组件，支持多种颜色和闪烁模式，适用于需要状态指示的场景。
#define LV_USE_LED        0

// 启用列表组件，提供一个用于显示项目列表的组件，支持多列、图标和事件处理等功能，适用于需要展示项目集合的场景。
#define LV_USE_LIST       0

// 启用菜单组件，提供一个用于显示层次化菜单的组件，支持多级菜单、图标和事件处理等功能，适用于需要复杂导航结构的场景。
#define LV_USE_MENU       0

// 启用消息框组件，提供一个用于显示消息和提示的对话框组件，支持多种按钮配置和事件处理，适用于需要用户确认或显示重要信息的场景。
#define LV_USE_METER      0

// 启用仪表组件，提供一个用于显示数值的仪表盘组件，支持多种样式和刻度配置，适用于需要显示实时数据的场景。
#define LV_USE_MSGBOX     0

// 启用预加载组件，提供一个用于显示加载状态的组件，支持多种动画和样式配置，适用于需要显示加载进度的场景。
#define LV_USE_SPAN       0
    #if LV_USE_SPAN
        /*A line text can contain maximum num of span descriptor */
        // 启用后，每行文本中可以包含的最大 span 描述符数量。span 描述符用于定义文本中的不同样式或颜色等属性。
        // 根据需要调整此值以满足你的文本样式需求，但请注意，过高的值可能会增加内存使用。
        #define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

// 启用预加载组件，提供一个用于显示加载状态的组件，支持多种动画和样式配置，适用于需要显示加载进度的场景。
#define LV_USE_SPINBOX    1

// 启用预加载组件，提供一个用于显示加载状态的组件，支持多种动画和样式配置，适用于需要显示加载进度的场景。
#define LV_USE_SPINNER    1

// 启用标签页组件，提供一个用于显示多个标签页的组件，支持标签切换和事件处理等功能，适用于需要在有限空间内展示多个视图的场景。
#define LV_USE_TABVIEW    1

// 启用瓦片视图组件，提供一个用于显示多个页面的组件，支持页面切换和事件处理等功能，适用于需要在有限空间内展示多个视图的场景。
#define LV_USE_TILEVIEW   1

// 启用时间选择器组件，提供一个用于选择时间的组件，支持小时、分钟和秒的选择，适用于需要时间输入的场景。
#define LV_USE_WIN        1

/*-----------
 * Themes
 *----------*/

/*A simple, impressive and very complete theme*/
// 启用默认主题，提供一个简单、完整且视觉效果不错的主题。启用后，LVGL 将使用这个主题来渲染界面元素，适用于需要快速开发和统一外观的场景。
#define LV_USE_THEME_DEFAULT 0
#if LV_USE_THEME_DEFAULT

    /*0: Light mode; 1: Dark mode*/
    // 启用暗色模式，提供一个适合在低光环境下使用的主题配色方案。启用后，默认主题将使用暗色调来渲染界面元素，适用于需要暗色界面的应用。
    #define LV_THEME_DEFAULT_DARK 0

    /*1: Enable grow on press*/
    // 启用按压时放大效果，提供一个交互反馈，当用户按下界面元素时，元素会稍微放大以增强触感反馈。
    // 启用后，默认主题中的按钮等可交互元素将具有这个效果，适用于需要增强用户交互体验的场景。
    #define LV_THEME_DEFAULT_GROW 1

    /*Default transition time in [ms]*/
    // 默认过渡时间，定义界面元素状态变化时的动画过渡时间，单位为毫秒。
    // 启用后，默认主题中的动画效果将使用这个过渡时间，适用于需要平滑动画效果的场景。
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/*A very simple theme that is a good starting point for a custom theme*/
// 启用基本主题，提供一个非常简单的主题，适合作为自定义主题的起点。
// 启用后，LVGL 将使用这个基本主题来渲染界面元素，适用于需要从头开始设计界面外观的场景。
#define LV_USE_THEME_BASIC 0

/*A theme designed for monochrome displays*/
// 启用单色主题，提供一个专为单色显示屏设计的主题。
// 启用后，LVGL 将使用这个主题来渲染界面元素，适用于需要在单色显示屏上使用 LVGL 的应用。
#define LV_USE_THEME_MONO 0

/*-----------
 * Layouts
 *----------*/

/*A layout similar to Flexbox in CSS.*/
// 启用 Flex 布局，提供一个类似于 CSS 中 Flexbox 的布局系统，允许你以灵活的方式排列子对象，支持水平和垂直方向的对齐、间距和换行等功能。
#define LV_USE_FLEX 1

/*A layout similar to Grid in CSS.*/
// 启用 Grid 布局，提供一个类似于 CSS 中 Grid 的布局系统，允许你以网格的方式排列子对象，支持复杂的二维布局需求。
#define LV_USE_GRID 1

/*---------------------
 * 3rd party libraries
 *--------------------*/

/*File system interfaces for common APIs */

/*API for fopen, fread, etc*/
// 启用标准输入输出文件系统接口，提供对 fopen、fread 等标准 C 文件操作函数的支持，使 LVGL 可以直接使用这些函数进行文件系统操作。
// 启用后，你需要配置相关的路径和缓存设置，以确保文件系统功能正常工作。
#define LV_USE_FS_STDIO 0
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_STDIO_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_STDIO_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for open, read, etc*/
// 启用 POSIX 文件系统接口，提供对 open、read 等 POSIX 标准文件操作函数的支持，使 LVGL 可以直接使用这些函数进行文件系统操作。
// 启用后，你需要配置相关的路径和缓存设置，以确保文件系统功能
#define LV_USE_FS_POSIX 1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER 'S'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_POSIX_PATH "/storage"         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_POSIX_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for CreateFile, ReadFile, etc*/
// 启用 Windows 文件系统接口，提供对 CreateFile、ReadFile 等 Windows API 文件操作函数的支持，使 LVGL 可以直接使用这些函数进行文件系统操作。
// 启用后，你需要配置相关的路径和缓存设置，以确保文件系统功能正常工作。仅在 Windows 平台上启用此选项。
#define LV_USE_FS_WIN32 0
#if LV_USE_FS_WIN32
    #define LV_FS_WIN32_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_WIN32_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_WIN32_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for FATFS (needs to be added separately). Uses f_open, f_read, etc*/
// 启用 FATFS 文件系统接口，提供对 FATFS 库的支持，使 LVGL 可以直接使用 FATFS 的 f_open、f_read 等函数进行文件系统操作。FATFS 是一个流行的嵌入式文件系统库，适用于 SD 卡等存储设备。
// 启用后，你需要配置相关的路径和缓存设置，并确保 FATFS 库已正确集成到你的项目中，以确保文件系统功能正常工作。
#define LV_USE_FS_FATFS 0
#if LV_USE_FS_FATFS
    #define LV_FS_FATFS_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_FATFS_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for LittleFS (library needs to be added separately). Uses lfs_file_open, lfs_file_read, etc*/
// 启用 LittleFS 文件系统接口，提供对 LittleFS 库的支持，使 LVGL 可以直接使用 LittleFS 的 lfs_file_open、lfs_file_read 等函数进行文件系统操作。
// LittleFS 是一个适用于嵌入式系统的轻量级文件系统库，具有高性能和可靠性。
// 启用后，你需要配置相关的路径和缓存设置，并确保 LittleFS 库已正确集成到你的项目中，以确保文件系统功能正常工作。
// 适用于需要在嵌入式设备上使用 LVGL 的应用。
#define LV_USE_FS_LITTLEFS 0
#if LV_USE_FS_LITTLEFS
    #define LV_FS_LITTLEFS_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_LITTLEFS_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*PNG decoder library*/
// 启用 PNG 解码库，提供对 PNG 图像格式的支持，使 LVGL 可以直接加载和显示 PNG 格式的图像。
// 启用后，你需要确保 PNG 解码库已正确集成到你的项目中，以确保图像解码功能正常工作。
#define LV_USE_PNG 0

/*BMP decoder library*/
// 启用 BMP 解码库，提供对 BMP 图像格式的支持，使 LVGL 可以直接加载和显示 BMP 格式的图像。
// BMP 是一种简单的位图图像格式，适用于需要基本图像支持的场景。
// 启用后，你需要确保 BMP 解码库已正确集成到你的项目中，以确保图像解码功能正常工作。
#define LV_USE_BMP 0

/* JPG + split JPG decoder library.
 * Split JPG is a custom format optimized for embedded systems. */
// 启用 JPG 和分割 JPG 解码库，提供对 JPG 图像格式的支持，使 LVGL 可以直接加载和显示 JPG 格式的图像。
// 分割 JPG 是一种针对嵌入式系统优化的自定义格式，可以在内存受限的环境中更高效地处理大图像。
// 启用后，你需要确保 JPG 解码库已正确集成到你的项目中，以确保图像解码功能正常工作。适用于需要支持 JPG 图像的应用。
#define LV_USE_SJPG 1

/*GIF decoder library*/
// 启用 GIF 解码库，提供对 GIF 图像格式的支持，使 LVGL 可以直接加载和显示 GIF 格式的图像。
// GIF 是一种支持动画的图像格式，适用于需要展示动态内容的场景。
// 启用后，你需要确保 GIF 解码库已正确集成到你的项目中，以确保图像解码功能正常工作。
// 适用于需要支持 GIF 图像的应用。
#define LV_USE_GIF 0

/*QR code library*/
// 启用 QR 码库，提供对 QR 码的生成和显示支持，使 LVGL 可以直接生成和显示 QR 码图像。适用于需要在界面上展示 QR 码的应用，如支付码、链接等。
#define LV_USE_QRCODE 0

/*FreeType library*/
// 启用 FreeType 库，提供对 TrueType 和 OpenType 字体的支持，使 LVGL 可以直接加载和使用这些字体格式。
// FreeType 是一个流行的开源字体引擎，适用于需要丰富字体支持的应用。
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
    /*Memory used by FreeType to cache characters [bytes] (-1: no caching)*/
    #define LV_FREETYPE_CACHE_SIZE (16 * 1024)
    #if LV_FREETYPE_CACHE_SIZE >= 0
        /* 1: bitmap cache use the sbit cache, 0:bitmap cache use the image cache. */
        /* sbit cache:it is much more memory efficient for small bitmaps(font size < 256) */
        /* if font size >= 256, must be configured as image cache */
        #define LV_FREETYPE_SBIT_CACHE 0
        /* Maximum number of opened FT_Face/FT_Size objects managed by this cache instance. */
        /* (0:use system defaults) */
        #define LV_FREETYPE_CACHE_FT_FACES 0
        #define LV_FREETYPE_CACHE_FT_SIZES 0
    #endif
#endif

/*Tiny TTF library*/
// 启用 Tiny TTF 库，提供对 TrueType 字体的支持，使 LVGL 可以直接加载和使用 TTF 字体格式。
// Tiny TTF 是一个轻量级的字体引擎，适用于资源受限的环境。
#define LV_USE_TINY_TTF 0
#if LV_USE_TINY_TTF
    /*Load TTF data from files*/
    #define LV_TINY_TTF_FILE_SUPPORT 0
#endif

/*Rlottie library*/
// 启用 Rlottie 库，提供对 Lottie 动画的支持，使 LVGL 可以直接加载和显示 Lottie 格式的动画。
// Lottie 是一种基于 JSON 的动画格式，适用于需要展示复杂动画的场景。
#define LV_USE_RLOTTIE 0

/*FFmpeg library for image decoding and playing videos
 *Supports all major image formats so do not enable other image decoder with it*/
// 启用 FFmpeg 库，提供对图像解码和视频播放的支持，使 LVGL 可以直接使用 FFmpeg 来处理各种图像格式和视频内容。
// 启用后，FFmpeg 将成为主要的图像解码器，因此不需要启用其他图像解码库。
#define LV_USE_FFMPEG 0
#if LV_USE_FFMPEG
    /*Dump input information to stderr*/
    #define LV_FFMPEG_DUMP_FORMAT 0
#endif

/*-----------
 * Others
 *----------*/

/*1: Enable API to take snapshot for object*/
// 启用对象快照功能，提供一个 API 来为对象拍摄快照，使你可以将对象的当前状态保存为图像。
// 适用于需要捕捉界面元素状态的场景，如调试或生成动态内容。
#define LV_USE_SNAPSHOT 0

/*1: Enable Monkey test*/
// 启用 Monkey 测试，提供一个随机事件生成器，用于对界面进行压力测试和稳定性测试。
// 启用后，Monkey 测试将随机生成各种事件（如点击、拖动等）来模拟用户交互，适用于需要验证界面稳定性的场景。
#define LV_USE_MONKEY 0

/*1: Enable grid navigation*/
// 启用网格导航功能，提供一个基于网格的导航系统，使用户可以通过键盘或其他输入设备在界面元素之间进行导航。
// 启用后，用户可以使用方向键或其他导航输入来在界面元素之间移动焦点，适用于需要键盘导航的应用，如电视界面等。
#define LV_USE_GRIDNAV 0

/*1: Enable lv_obj fragment*/
// 启用 lv_obj 片段功能，提供一个用于创建和管理界面元素片段的 API，使你可以将界面元素分成更小的部分进行处理和优化。
// 启用后，你可以使用 lv_obj 片段来管理界面元素的部分更新和渲染，适用于需要优化界面性能的场景，如复杂界面或动画等。
#define LV_USE_FRAGMENT 0

/*1: Support using images as font in label or span widgets */
// 启用图像字体支持，允许在标签或 span 组件中使用图像作为字体，使你可以创建更具视觉效果的文本显示。
// 启用后，你可以将图像资源用作文本的字体，适用于需要特殊文本效果的场景，如使用图标字体或自定义字符集等。
#define LV_USE_IMGFONT 0

/*1: Enable a published subscriber based messaging system */
// 启用基于发布-订阅的消息系统，提供一个用于组件之间通信的机制，使不同组件可以通过发布和订阅消息来进行交互和数据传递。
// 启用后，你可以使用这个消息系统来实现组件之间的松耦合通信，适用于需要组件间交互的复杂应用，如事件驱动的界面等。
#define LV_USE_MSG 0

/*1: Enable Pinyin input method*/
/*Requires: lv_keyboard*/
// 启用拼音输入法，提供一个用于输入中文拼音的界面，使用户可以通过键盘输入拼音来选择相应的汉字。
#define LV_USE_IME_PINYIN 0
#if LV_USE_IME_PINYIN
    /*1: Use default thesaurus*/
    /*If you do not use the default thesaurus, be sure to use `lv_ime_pinyin` after setting the thesauruss*/
    // 启用默认词库，提供一个预定义的拼音词库，使拼音输入法可以直接使用这些词库来进行汉字选择。
    // 启用后，拼音输入法将使用默认的词库来提供汉字选择功能，适用于需要快速启用拼音输入法的场景。
    // 如果你需要使用自定义词库，可以将此选项设置为 0，并在代码中使用 `lv_ime_pinyin` 来加载你的词库。
    #define LV_IME_PINYIN_USE_DEFAULT_DICT 1
    /*Set the maximum number of candidate panels that can be displayed*/
    /*This needs to be adjusted according to the size of the screen*/
    // 设置可以显示的候选面板的最大数量。根据屏幕的大小调整此值，以确保输入法界面能够适应你的设备。
    #define LV_IME_PINYIN_CAND_TEXT_NUM 6

    /*Use 9 key input(k9)*/
    // 启用 9 键输入（k9），提供一个适用于小屏幕设备的输入方式，使用户可以通过类似于传统手机键盘的方式输入拼音。
    #define LV_IME_PINYIN_USE_K9_MODE      0
    #if LV_IME_PINYIN_USE_K9_MODE == 1
        // 设置 9 键输入模式下可以显示的候选文本的最大数量。根据屏幕的大小和输入法界面的设计调整此值，以确保输入法界面能够适应你的设备。
        #define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
    #endif // LV_IME_PINYIN_USE_K9_MODE
#endif

/*==================
* EXAMPLES
*==================*/

/*Enable the examples to be built with the library*/
// 启用示例，允许在构建库时包含示例代码，这些示例可以帮助你了解如何使用 LVGL 的各种功能和组件。
// 启用后，你可以在项目中找到这些示例并进行测试和学习。
#define LV_BUILD_EXAMPLES 0

/*===================
 * DEMO USAGE
 ====================*/

/*Show some widget. It might be required to increase `LV_MEM_SIZE` */
// 启用演示组件，提供一些预定义的界面示例，展示 LVGL 的各种组件和功能。
// 启用后，你可以运行这些示例来了解如何使用 LVGL 来构建界面。
#define LV_USE_DEMO_WIDGETS 0
#if LV_USE_DEMO_WIDGETS
#define LV_DEMO_WIDGETS_SLIDESHOW 0
#endif

/*Demonstrate the usage of encoder and keyboard*/
// 启用键盘和编码器演示，提供一个示例界面，展示如何使用键盘和编码器进行交互。
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0

/*Benchmark your system*/
// 启用基准测试，提供一个用于测试系统性能的示例界面，可以帮助你评估 LVGL 在你的平台上的表现。
#define LV_USE_DEMO_BENCHMARK 0
#if LV_USE_DEMO_BENCHMARK
/*Use RGB565A8 images with 16 bit color depth instead of ARGB8565*/
#define LV_DEMO_BENCHMARK_RGB565A8 0
#endif

/*Stress test for LVGL*/
// 启用压力测试，提供一个用于对 LVGL 进行压力测试的示例界面，可以帮助你评估 LVGL 在极端条件下的稳定性和性能。
#define LV_USE_DEMO_STRESS 0

/*Music player demo*/
// 启用音乐播放器演示，提供一个示例界面，展示如何使用 LVGL 来构建一个简单的音乐播放器界面。
#define LV_USE_DEMO_MUSIC 0
#if LV_USE_DEMO_MUSIC
    // 启用不同风格的音乐播放器界面示例，如方形、横向、圆形和大型界面等。根据需要启用所需的示例。
    #define LV_DEMO_MUSIC_SQUARE    0
    // 启用横向音乐播放器界面示例，展示一个适合在宽屏设备上使用的音乐播放器界面。
    #define LV_DEMO_MUSIC_LANDSCAPE 0
    // 启用圆形音乐播放器界面示例，展示一个适合在圆形显示屏上使用的音乐播放器界面。
    #define LV_DEMO_MUSIC_ROUND     0
    // 启用大型音乐播放器界面示例，展示一个适合在大屏设备上使用的音乐播放器界面。
    #define LV_DEMO_MUSIC_LARGE     0
    // 启用自动播放功能，在音乐播放器示例中自动开始播放音乐，适用于需要展示完整音乐播放器功能的场景。
    #define LV_DEMO_MUSIC_AUTO_PLAY 0
#endif

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
