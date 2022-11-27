#pragma once
#include "pico/stdlib.h"

typedef enum {ON=1, OFF=0} pixel_state_t;

void pio_display_init();
uint8_t *get_display(const uint8_t i);
void fill(uint8_t * const fb, const uint8_t pattern);
void clear(uint8_t * const fb);
void pixel(uint8_t * const fb, const uint8_t x, const uint8_t y, const pixel_state_t on);
void pio_display_update();
void pio_display_wait_for_finish_blocking();
