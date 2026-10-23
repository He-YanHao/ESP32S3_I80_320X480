idf_component_register(
    SRCS "bsp_touch.c"
    INCLUDE_DIRS "."
    REQUIRES
        bsp_exio
        freertos
    PRIV_REQUIRES
        esp_driver_i2c
        esp_driver_gpio
        esp_common
)