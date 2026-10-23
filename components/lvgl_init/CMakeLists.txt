idf_component_register(
    SRCS "lvgl_init.c"
    INCLUDE_DIRS "."
    REQUIRES 
        bsp_lcd 
        bsp_touch 
        lvgl__lvgl
        esp_lcd
        espressif__esp_lvgl_adapter
    PRIV_REQUIRES 
        esp_timer 
        guider_ui
)