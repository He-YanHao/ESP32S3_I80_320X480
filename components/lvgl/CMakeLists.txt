file(GLOB_RECURSE LVGL_SOURCES "src/*.c")

idf_component_register(
    SRCS ${LVGL_SOURCES}
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "src"
    REQUIRES esp_timer freertos
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    LV_LVGL_H_INCLUDE_SIMPLE
)