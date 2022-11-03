add_executable(display_board)

pico_generate_pio_header(display_board ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

target_sources(display_board PRIVATE pio-display.c)

target_link_libraries(display_board PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        pico_time
        )

pico_add_extra_outputs(display_board)
