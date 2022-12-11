add_library(midi)

target_sources(midi PRIVATE midi.c)

target_link_libraries(midi PRIVATE pico_stdlib pico_util hardware_uart)

target_include_directories(midi PUBLIC include/)
