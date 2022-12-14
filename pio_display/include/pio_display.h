#pragma once
#include "pico/stdlib.h"

typedef enum {SIZE_13 = 0, SIZE_18 = 1, SIZE_28 = 2, SIZE_32 = 3} pio_display_font_size_t;
typedef struct {
    uint8_t width;
    uint8_t height;
} pio_display_box_t;

void pio_display_init();
uint8_t *pio_display_get(const uint8_t i);
void pio_display_fill(uint8_t * const fb, const uint8_t pattern);
void pio_display_clear(uint8_t * const fb);
void pio_display_pixel(uint8_t * const fb, const uint8_t x, const uint8_t y, const bool on);
void pio_display_fill_rectangle(uint8_t * const fb,
                                const uint8_t startx, const uint8_t starty,
                                const uint8_t endx, const uint8_t endy);
void pio_display_printc(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const pio_display_font_size_t font_size, const bool on, const char c);
void pio_display_print(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const pio_display_font_size_t font_size, const bool on, const char * const str);
pio_display_box_t text_box(const pio_display_font_size_t font_size, const char * const str);
void pio_display_print_center(uint8_t * const fb, const uint8_t y, const pio_display_font_size_t font_size, const bool on, const char * const str);
void pio_display_clear_current_framebuffer();
void pio_display_update_and_flip();
void pio_display_wait_for_finish_blocking();
bool pio_display_can_wait_without_blocking();
