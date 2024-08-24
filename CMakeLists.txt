cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(display_board)

pico_sdk_init()

add_subdirectory(./lib)
add_subdirectory(./i2c_controller)
add_subdirectory(./pio_display)
add_subdirectory(./sdhi)
add_subdirectory(./midi)
add_subdirectory(./action)
add_subdirectory(./setup)
add_subdirectory(./ymf262)
add_subdirectory(./ymf262_synth)
add_subdirectory(./dsl)
add_subdirectory(./src)
