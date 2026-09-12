file(GLOB_RECURSE LVGL_SOURCES "src/*.c")

idf_component_register(
    SRCS ${LVGL_SOURCES}
    INCLUDE_DIRS "." "src"
    PRIV_INCLUDE_DIRS ""
    REQUIRES esp_timer freertos driver
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    LV_LVGL_H_INCLUDE_SIMPLE
)