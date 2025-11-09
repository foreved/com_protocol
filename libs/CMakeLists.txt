cmake_minimum_required(VERSION 3.22)
enable_language(C)

add_library(com_protocol STATIC)
target_sources(com_protocol PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mod_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mod_oled.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_tool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mod_dht11.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_font_fixedsys.c
)
target_include_directories(com_protocol
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
)
target_link_libraries(com_protocol PRIVATE STM32_Drivers)
