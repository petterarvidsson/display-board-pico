add_executable(display_board)

target_sources(display_board PRIVATE main.c)

target_link_libraries(display_board PRIVATE
        pico_stdlib
        pico_multicore
        pio_display
        i2c_controller
        sdhi
        midi
        action
        setup
        drum
        pico_time
        )

pico_add_extra_outputs(display_board)
