cmake_minimum_required(VERSION 3.22)
enable_language(C)

add_library(fatfs STATIC)
target_include_directories(fatfs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/.
)
target_sources(fatfs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/diskio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ffsystem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ffunicode.c
)
target_link_libraries(fatfs PRIVATE STM32_Drivers com_protocol)