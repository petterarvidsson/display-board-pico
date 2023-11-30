#include "pio_display.h"
#include <stdlib.h>
#include <math.h>
/* Include all fonts */
#include <fonts.inc>

static uint8_t font_bytes[] = {1, 1, 2, 2};
static uint8_t font_width[] = {7, 8, 16, 16};
static uint8_t font_height[] = {13, 18, 28, 32};
static uint8_t font_offset[] = {13, 18, 28*2, 32*2};
static uint8_t *fonts[] = {font_13, font_18, font_28, NULL};

static void safe_pixel(uint8_t * const fb, const int16_t x, const int16_t y, const bool on) {
  const uint8_t x_safe = x < 0 ? 0 : x > 127 ? 127 : x;
  const uint8_t y_safe = y < 0 ? 0 : y > 63 ? 63 : y;
  pio_display_pixel(fb, x_safe, y_safe, on);
}


static void pixel(uint8_t * const fb, uint8_t x, uint8_t y, uint8_t size) {
  const uint8_t x_min = x - size < 0 ? 0 : x - size;
  const uint8_t x_max = x + size > 127 ? 127 : x + size;
  const uint8_t y_min = y - size < 0 ? 0 : y - size;
  const uint8_t y_max = y + size > 63 ? 63 : y + size;

  for(uint8_t xi = x_min; xi <= x_max ; xi++) {
    for(uint8_t yi = y_min; yi <= y_max; yi++) {
      pio_display_pixel(fb, xi, yi, true);
    }
  }
}

static void draw_line(uint8_t * const fb, uint8_t x0, uint8_t y0, const uint8_t x1, const uint8_t y1, const uint8_t size) {
  int16_t sx = x0 < x1 ? 1 : -1;
  int16_t sy = y0 < y1 ? 1 : -1;
  int16_t dx =  abs(x1 - x0);
  int16_t dy = -abs(y1 - y0);
  int16_t err = dx + dy;
  int16_t e2;

  while(x0 != x1 || y0 != y1) {
    pixel(fb, x0, y0, size);
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
  pixel(fb, x0, y0, size);
}

static void draw_circle(uint8_t * const fb, const uint8_t x0, const uint8_t y0, const uint8_t radius) {
  int f = 1 - radius;
  int ddf_x = 1;
  int ddf_y = -2 * radius;
  int x = 0;
  int y = radius;

  safe_pixel(fb, x0, y0 + radius, true);
  safe_pixel(fb, x0, y0 - radius, true);
  safe_pixel(fb, x0 + radius, y0, true);
  safe_pixel(fb, x0 - radius, y0, true);
  while (x < y) {
    if(f >= 0) {
      y--;
      ddf_y += 2;
      f += ddf_y;
    }
    x++;
    ddf_x += 2;
    f += ddf_x;
    safe_pixel(fb, x0 + x, y0 + y, true);
    safe_pixel(fb, x0 - x, y0 + y, true);
    safe_pixel(fb, x0 + x, y0 - y, true);
    safe_pixel(fb, x0 - x, y0 - y, true);
    safe_pixel(fb, x0 + y, y0 + x, true);
    safe_pixel(fb, x0 - y, y0 + x, true);
    safe_pixel(fb, x0 + y, y0 - x, true);
    safe_pixel(fb, x0 - y, y0 - x, true);
  }
}

static void draw_sine(uint8_t * const fb, const uint8_t from, const uint8_t to, const uint8_t x, const uint8_t y, const uint8_t length, const uint8_t amplitude) {
  const float ffrom = (float)from * (M_PI / 8);
  const float interval = (float)to * (M_PI / 8) - ffrom;
  uint8_t old_yi = y + (int8_t)(sinf(ffrom) * amplitude);
  for(uint8_t i = 1; i < length; i++) {
    uint8_t xi = x + i;
    int16_t yi = y + (int16_t)(sinf(ffrom + interval * ((float)i / (float)length)) * amplitude);
    draw_line(fb, xi - 1, old_yi, xi, yi, 0);
    old_yi = yi;
  }
}

static void fill_x(uint8_t * const fb, const int16_t x, const int16_t y, const uint8_t length) {
  for(uint8_t i = 0; i < length; i++) {
    const int16_t xi = x + i;
    if(xi > 0 && xi <= 127) {
      pio_display_pixel(fb, xi, y, true);
    }
  }
}

static void draw_filled_circle(uint8_t * const fb, const uint8_t x0, const uint8_t y0, const uint8_t radius) {
  int f = 1 - radius;
  int ddf_x = 1;
  int ddf_y = -2 * radius;
  int x = 0;
  int y = radius;

  safe_pixel(fb, x0, y0 + radius, true);
  safe_pixel(fb, x0, y0 - radius, true);
  safe_pixel(fb, x0 + radius, y0, true);
  safe_pixel(fb, x0 - radius, y0, true);
  fill_x(fb, x0 - radius, y0, radius * 2);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddf_y += 2;
      f += ddf_y;
    }
    x++;
    ddf_x += 2;
    f += ddf_x;
    safe_pixel(fb, x0 + x, y0 + y, true);
    safe_pixel(fb, x0 - x, y0 + y, true);
    fill_x(fb, x0 - x, y0 + y, x * 2);
    safe_pixel(fb, x0 + x, y0 - y, true);
    safe_pixel(fb, x0 - x, y0 - y, true);
    fill_x(fb, x0 - x, y0 - y, x * 2);
    safe_pixel(fb, x0 + y, y0 + x, true);
    safe_pixel(fb, x0 - y, y0 + x, true);
    fill_x(fb, x0 - y, y0 + x, y * 2);
    safe_pixel(fb, x0 + y, y0 - x, true);
    safe_pixel(fb, x0 - y, y0 - x, true);
    fill_x(fb, x0 - y, y0 - x, y * 2);
  }
}

static void display_list_item(const uint8_t x_offset, const uint8_t y_offset, const display_list_item_t item, const uint8_t display) {
  uint8_t * const fb = pio_display_get(display);
  switch(item.type) {
  case DISPLAY_LIST_LINE:
    {
      const display_list_item_line_t line = item.configuration.line;
      draw_line(fb, line.start.x + x_offset, line.start.y + y_offset, line.end.x  + x_offset, line.end.y  + y_offset, line.size);
    }
    break;
  case DISPLAY_LIST_CIRCLE:
    {
      const display_list_item_circle_t circle = item.configuration.circle;
      draw_circle(fb, circle.center.x + x_offset, circle.center.y + y_offset, circle.radius);
    }
    break;
  case DISPLAY_LIST_FILLED_CIRCLE:
    {
      const display_list_item_circle_t circle = item.configuration.circle;
      draw_filled_circle(fb, circle.center.x + x_offset, circle.center.y + y_offset, circle.radius);
    }
    break;
  case DISPLAY_LIST_SINE_INTERVAL:
    {
      const display_list_item_sine_interval_t sine = item.configuration.sine_interval;
      draw_sine(fb, sine.from, sine.until, sine.start.x + x_offset, sine.start.y + y_offset, sine.length, sine.amplitude);
    }
    break;
  }
}

void pio_display_list(const uint8_t x_offset, const uint8_t y_offset, const display_list_t list, const uint8_t display) {
  for(uint8_t i = 0; i < list.size; i++) {
    display_list_item(x_offset, y_offset, list.items[i], display);
  }
};

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

box_t pio_display_text_box(const pio_display_font_size_t font_size, const char * const str) {
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

    const box_t box = {width, font_height[font_size]};
    return box;
}

static uint8_t center_box_x(const box_t box) {
    return (128 - box.width) / 2;
}

static uint8_t center_box_y(const box_t box) {
    return (64 - box.height) / 2;
}

void pio_display_print_center(uint8_t * const fb, const uint8_t y, const pio_display_font_size_t font_size, const bool on, const char * const str) {
    const box_t box = pio_display_text_box(font_size, str);
    if(box.width < 128) {
        uint8_t offset = center_box_x(box);
        pio_display_print(fb, offset, y, font_size, on, str);
    }
}
