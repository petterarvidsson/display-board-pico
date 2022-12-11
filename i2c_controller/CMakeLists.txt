add_library(i2c_controller)

target_sources(i2c_controller PRIVATE i2c_controller.c)

target_link_libraries(i2c_controller PRIVATE pico_stdlib pico_sync hardware_i2c)

target_include_directories(i2c_controller PUBLIC include/)
