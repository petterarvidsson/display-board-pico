cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(display-board)

pico_sdk_init()

add_subdirectory(./pio-display)
add_subdirectory(./src)
