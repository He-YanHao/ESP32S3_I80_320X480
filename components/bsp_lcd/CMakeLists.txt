idf_component_register(
    SRCS "bsp_lcd.c"
    INCLUDE_DIRS "."
    REQUIRES bsp_exio esp_lcd_st7796 esp_lcd
)