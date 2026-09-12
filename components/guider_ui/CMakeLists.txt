# 使用 CMake 的 GLOB 命令自动查找所有 .c 文件
file(GLOB GUI_GUIDER_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/custom/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/guider_fonts/*.c"
    # 如果以后有图片，可以取消下面这行的注释
    # "${CMAKE_CURRENT_SOURCE_DIR}/generated/images/*.c"
)

# 将找到的文件列表传给 idf_component_register
idf_component_register(
    SRCS ${GUI_GUIDER_SRCS}
    INCLUDE_DIRS
        "custom"
        "generated"
    REQUIRES
        lvgl
)