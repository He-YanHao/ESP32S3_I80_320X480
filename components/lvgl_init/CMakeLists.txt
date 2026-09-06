idf_component_register(
    SRCS "lvgl_init.c"
    INCLUDE_DIRS "."
    REQUIRES bsp_lcd lvgl esp_lcd
    PRIV_REQUIRES esp_timer
)