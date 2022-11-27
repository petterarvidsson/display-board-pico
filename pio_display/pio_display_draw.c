#include "pio_display.h"

/* Include all fonts */
//#include <fonts.inc>

void pio_display_fill_rectangle(uint8_t * const fb,
                                const uint8_t startx, const uint8_t starty,
                                const uint8_t endx, const uint8_t endy) {
  for(uint8_t x = startx; x <= endx; x++) {
    for(uint8_t y = starty; y <= endy; y++) {
      pio_display_pixel(fb, x, y, true);
    }
  }
}

/* void printc(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const font_size_t font_size, const pixel_state_t on, const char c) { */
/*     uint32_t index = (uint32_t)c * font_offset[font_size]; */
/*     for(uint8_t i = 0; i < font_height[font_size]; i++) { */
/*         uint8_t y = starty + i; */
/*         for(uint8_t b = 0; b < font_bytes[font_size]; b++) { */
/*             uint8_t segment = fonts[font_size][index + (i * font_bytes[font_size]) + b]; */
/*             for(uint8_t j = 0; j < 8; j++) { */
/*                 uint8_t p = (segment >> j) & 0x01; */
/*                 if(p) { */
/*                     uint8_t x = startx + (8 - j) + (8 * b); */
/*                     pixel(fb, x, y, on); */
/*                 } */
/*             } */
/*         } */
/*     } */
/* } */

/* void print(uint8_t * const fb, const uint8_t startx, const uint8_t starty, const font_size_t font_size, const pixel_state_t on, const char * const str) { */
/*     const char * c = str; */
/*     uint8_t x = startx; */
/*     while (*c) { */
/*         const char chr = *c++; */
/*         if(chr != ' ') { */
/*             printc(fb, x, starty, font_size, on, chr); */
/*             x += font_bytes[font_size]*8; */
/*         } else { */
/*             x += 8; */
/*         } */
/*     } */
/* } */

/* box_t text_box(const font_size_t font_size, const char * const str) { */
/*     const char * c = str; */
/*     uint8_t len = 0; */
/*     while (*c) { */
/*         const char chr = *c++; */
/*         if(chr != ' ') { */
/*             len += font_bytes[font_size]; */
/*         } else { */
/*             len += 1; */
/*         } */
/*     } */
/*     const uint8_t width = 8 * len; */

/*     const box_t box = {width, font_height[font_size]}; */
/*     return box; */
/* } */

/* uint8_t center_box_x(const box_t box) { */
/*     return (128 - box.width) / 2; */
/* } */

/* uint8_t center_box_y(const box_t box) { */
/*     return (64 - box.height) / 2; */
/* } */

/* void print_center(uint8_t * const fb, const uint8_t y, const font_size_t font_size, const pixel_state_t on, const char * const str) { */
/*     const box_t box = text_box(font_size, str); */
/*     if(box.width < 128) { */
/*         uint8_t offset = center_box_x(box); */
/*         print(fb, offset, y, font_size, on, str); */
/*     } */
/* } */
