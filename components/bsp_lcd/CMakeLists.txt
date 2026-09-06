idf_component_register(
    SRCS "bsp_lcd.c"
    INCLUDE_DIRS "."
    REQUIRES exio st7796 esp_lcd
)