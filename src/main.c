#include "pico/stdlib.h"
#include "stdio.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"

enum controls {
  NONE = -1,
  TEST = 0,
  TEST2,
  TEST3,
  CONTROLS
};

static const sdhi_control_t const controls[] = {
  {
    .id = TEST,
    .title = "test control",
    .group = 0,
    .min = 0,
    .max = 127
  },
  {
    .id = TEST2,
    .title = "test control2",
    .group = 0,
    .min = 0,
    .max = 64
  },
  {
    .id = TEST3,
    .title = "test control2",
    .group = 1,
    .min = 0,
    .max = 64
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t * const groups = NULL;
static const sdhi_panel_t const panels[] = {
  {
    "Test",
    {
      TEST, TEST2, NONE,
      NONE, TEST3, NONE,
      NONE, NONE
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
static int32_t values[CONTROLS] = {0, 0, 0};

int main() {
    stdio_init_all();
    pio_display_init();
    i2c_controller_init();
    sdhi_init(sdhi);
    absolute_time_t start;
    absolute_time_t end;
    uint32_t us = 0;
    start = get_absolute_time();
    pio_display_update_and_flip();
    sdhi_update_displays(values, sdhi);
    for(uint32_t i = 0;;) {
      if(pio_display_can_wait_without_blocking()) {
        pio_display_wait_for_finish_blocking();
        end = get_absolute_time();
        i++;
        us += absolute_time_diff_us(start, end);
        start = get_absolute_time();
        pio_display_update_and_flip();
        sdhi_update_displays(values, sdhi);
      }
      sdhi_update_values_blocking(values, sdhi);
    }
}
