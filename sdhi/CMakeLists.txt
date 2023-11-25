add_library(sdhi)

target_sources(sdhi PRIVATE sdhi.c)

target_link_libraries(sdhi PUBLIC i2c_controller lib PRIVATE pico_stdlib pio_display)

target_include_directories(sdhi PUBLIC include/)
