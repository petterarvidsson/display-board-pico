#include "pico/stdlib.h"
#include "stdio.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"

#define C_NONE -1
#define C_TEST 0
static const sdhi_control_t const controls[] = {
  {
    .id = C_TEST,
    .title = "test control",
    .group = 0,
    .initial = 0,
    .min = 0,
    .max = 127
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t * const groups = NULL;
static const sdhi_panel_t const panels[] = {
  {
    "Test",
    {
      C_TEST, C_NONE, C_NONE,
      C_NONE, C_NONE, C_NONE,
      C_NONE, C_NONE
    }
  }
};
static const uint32_t panels_size = sizeof(panels) / sizeof(sdhi_panel_t);
static sdhi_t sdhi = {
  controls,
  controls_size,
  groups,
  0,
  panels,
  panels_size
};
static int32_t values[C_TEST + 1] = {0};

int main() {
    stdio_init_all();
    pio_display_init();
    i2c_controller_init();
    sdhi_init(sdhi);
    absolute_time_t start;
    absolute_time_t end;
    uint32_t us = 0;
    start = get_absolute_time();
    uint8_t x = 0, y = 0;
    int8_t dx = 1, dy = 1;
    pio_display_update();
    for(uint32_t i = 0;;) {
      if(pio_display_can_wait_without_blocking()) {
        pio_display_wait_for_finish_blocking();
        end = get_absolute_time();
        i++;
        us += absolute_time_diff_us(start, end);
        if(i % 1 == 0) {
          for(uint8_t i = 0; i < 40; i++) {
            uint8_t *display = pio_display_get(i);
            pio_display_clear(display);
            //pio_display_fill_rectangle(display, x, y, x + 4, y + 4);
            //pio_display_print(display, x, y, SIZE_13, true, "PQ");
            //pio_display_fill_rectangle(display, 0, 0, 4, 4);
            pio_display_print_center(display, y,  SIZE_13, true, "Hello world!");
          }
          x = x + dx;
          y = y + dy;
          if(x > 108) {
          dx = -1;
          }
          if(x < 1) {
            dx = 1;
          }
          if(y > 44) {
            dy = -1;
          }
          if(y < 1) {
            dy = 1;
          }
        }                                       \
        start = get_absolute_time();
        pio_display_update();
      }
      sdhi_update_values_blocking(values, sdhi);
    }
}
