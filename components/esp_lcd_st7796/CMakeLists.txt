idf_component_register(
    SRCS "esp_lcd_st7796.c" "esp_lcd_st7796_general.c"
    INCLUDE_DIRS "include" "."
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES esp_lcd
    PRIV_REQUIRES esp_driver_gpio
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    -DESP_LCD_ST7796_VER_MAJOR=1
    -DESP_LCD_ST7796_VER_MINOR=4
    -DESP_LCD_ST7796_VER_PATCH=0
)