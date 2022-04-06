cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(cs122a_project1 C CXX ASM)
pico_sdk_init()

add_executable(main
        main.c
        usb_descriptors.c
        src/utils.c
)
pico_add_extra_outputs(main)
target_include_directories(main PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(main PUBLIC pico_stdlib tinyusb_device tinyusb_board)
