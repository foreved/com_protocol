cmake_minimum_required(VERSION 3.22)
enable_language(C)

add_library(com_protocol STATIC)
target_sources(com_protocol PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mod_flash.c
)
target_include_directories(com_protocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(com_protocol PRIVATE STM32_Drivers)