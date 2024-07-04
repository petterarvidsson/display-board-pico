add_library(ymf262)

target_sources(ymf262 PRIVATE ymf262.c)

target_link_libraries(ymf262 PRIVATE pico_stdlib hardware_i2c action_pp sdhi midi setup)

target_include_directories(ymf262 PUBLIC include/)
