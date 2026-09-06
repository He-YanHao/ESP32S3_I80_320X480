idf_component_register(SRCS "main.c"
                       PRIV_REQUIRES spi_flash esp_psram bsp_exio esp_lcd_st7796 bsp_lcd
                       INCLUDE_DIRS "")
