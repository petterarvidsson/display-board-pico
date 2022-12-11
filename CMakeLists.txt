cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(display_board)

pico_sdk_init()

add_subdirectory(./i2c_controller)
add_subdirectory(./pio_display)
add_subdirectory(./sdhi)
add_subdirectory(./midi)
add_subdirectory(./src)
