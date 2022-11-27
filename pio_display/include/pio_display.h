#pragma once
#include "pico/stdlib.h"

void pio_display_init();
uint8_t *pio_display_get(const uint8_t i);
void pio_display_fill(uint8_t * const fb, const uint8_t pattern);
void pio_display_clear(uint8_t * const fb);
void pio_display_pixel(uint8_t * const fb, const uint8_t x, const uint8_t y, const bool on);
void pio_display_fill_rectangle(uint8_t * const fb,
                                const uint8_t startx, const uint8_t starty,
                                const uint8_t endx, const uint8_t endy);
void pio_display_update();
void pio_display_wait_for_finish_blocking();
bool pio_display_can_wait_without_blocking();
