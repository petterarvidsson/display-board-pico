add_executable(pio_display)

pico_generate_pio_header(pio_display ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

target_sources(pio_display PRIVATE pio-display.c)

target_link_libraries(pio_display PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        pico_time
        )

pico_add_extra_outputs(pio_display)
