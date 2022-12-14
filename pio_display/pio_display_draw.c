#include "pio_display.h"

/* Include all fonts */
#include <fonts.inc>

static uint8_t font_bytes[] = {1, 1, 2, 2};
static uint8_t font_width[] = {7, 8, 16, 16};
static uint8_t font_height[] = {13, 18, 28, 32};
static uint8_t font_offset[] = {13, 18, 28*2, 32*2};
static uint8_t *fonts[] = {font_13, font_18, font_28, NULL};

void pio_display_fill_rectangle(uint8_t * const fb,
                                const uint8_t startx, const uint8_t starty,
                                const uint8_t endx, const uint8_t endy) {
  for(uint8_t x = startx; x <= endx; x++) {
    for(uint8_t y = starty; y <= endy; y++) {
      pio_display_pixel(fb, x, y, true);
    }
  }
}

void pio_display_printc(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const pio_display_font_size_t font_size, const bool on, const char c) {
    uint32_t index = (uint32_t)c * font_offset[font_size];
    for(uint8_t i = 0; i < font_height[font_size]; i++) {
        uint8_t y = starty + font_height[font_size] - i;
        for(uint8_t b = 0; b < font_bytes[font_size]; b++) {
            uint8_t segment = fonts[font_size][index + (i * font_bytes[font_size]) + b];
            for(uint8_t j = 0; j < 8; j++) {
                uint8_t p = (segment >> j) & 0x01;
                if(p) {
                    uint8_t x = startx + (8 - j) + (8 * b);
                     pio_display_pixel(fb, x, y, on);
                }
            }
        }
    }
}

void pio_display_print(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const pio_display_font_size_t font_size, const bool on, const char * const str) {
    const char * c = str;
    uint8_t x = startx;
    while (*c) {
        const char chr = *c++;
        if(chr != ' ') {
            pio_display_printc(fb, x, starty, font_size, on, chr);
            x += font_bytes[font_size]*8;
        } else {
            x += 8;
        }
    }
}

pio_display_box_t pio_display_text_box(const pio_display_font_size_t font_size, const char * const str) {
    const char * c = str;
    uint8_t len = 0;
    while (*c) {
        const char chr = *c++;
        if(chr != ' ') {
            len += font_bytes[font_size];
        } else {
            len += 1;
        }
    }
    const uint8_t width = 8 * len;

    const pio_display_box_t box = {width, font_height[font_size]};
    return box;
}

static uint8_t center_box_x(const pio_display_box_t box) {
    return (128 - box.width) / 2;
}

static uint8_t center_box_y(const pio_display_box_t box) {
    return (64 - box.height) / 2;
}

void pio_display_print_center(uint8_t * const fb, const uint8_t y, const pio_display_font_size_t font_size, const bool on, const char * const str) {
    const pio_display_box_t box = pio_display_text_box(font_size, str);
    if(box.width < 128) {
        uint8_t offset = center_box_x(box);
        pio_display_print(fb, offset, y, font_size, on, str);
    }
}
