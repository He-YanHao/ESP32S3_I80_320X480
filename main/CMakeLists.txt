idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES spi_flash esp_psram bsp_exio lvgl_init fatfs
)