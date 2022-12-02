add_library(sdhi)

target_sources(sdhi PRIVATE sdhi.c)

target_link_libraries(sdhi PRIVATE pico_stdlib i2c_controller pio_display)

target_include_directories(sdhi PUBLIC include/)
