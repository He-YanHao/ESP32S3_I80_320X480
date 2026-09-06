idf_component_register(
    SRCS "xl9555.c" "bsp_exio.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_i2c
)