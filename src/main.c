#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "stdio.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"
#include "midi.h"

enum controls {
  NONE = -1,
  TEST1 = 0,
  TEST2,
  TEST3,
  TEST4,
  TEST5,
  TEST6,
  TEST7,
  TEST8,
  TEST9,
  CONTROLS
};

static const char * const enum_values[] = {
  "Snare",
  "Bass",
  "Hi-hat"
};

static const sdhi_control_t const controls[] = {
  {
    .id = TEST1,
    .title = "Drum",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = enum_values,
      .size = sizeof(enum_values) / sizeof(char *)
    }
  },
  {
    .id = TEST2,
    .title = "test control2",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST3,
    .title = "test control3",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST4,
    .title = "test control4",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST5,
    .title = "test control5",
    .group = 1,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST6,
    .title = "test control6",
    .group = 1,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST7,
    .title = "test control7",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST8,
    .title = "test control8",
    .group = 3,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 64
    }
  },
  {
    .id = TEST9,
    .title = "test control9",
    .group = 1,
    .type = SDHI_CONTROL_TYPE_REAL,
    .configuration.real = {
      .min = -14.5,
      .max = 14.5,
      .step = 0.5
    }
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t * const groups = NULL;
static const sdhi_panel_t const panels[] = {
  {
    "Panel 1",
    {
      TEST1, TEST2, TEST7,
      TEST4, TEST3, TEST8,
      TEST5, TEST6
    }
  },
  {
    "Panel 2",
    {
      NONE,  NONE,  NONE,
      TEST9, NONE,  NONE,
      NONE,  NONE
    }
  }
};
static const uint32_t panels_size = sizeof(panels) / sizeof(sdhi_panel_t);
static sdhi_t sdhi = {
  controls,
  controls_size,
  groups,
  0,
  "Change panel",
  panels,
  panels_size
};
static int32_t values[CONTROLS] = {0, 0, 0};

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

int main() {
  stdio_init_all();
  pio_display_init();
  i2c_controller_init();
  sdhi_init(sdhi);
  midi_init();
  multicore_launch_core1(real_time);

  absolute_time_t start;
  absolute_time_t end;
  uint32_t us = 0;
  start = get_absolute_time();
  pio_display_update_and_flip();
  sdhi_update_displays(values, sdhi);
  printf("Start MIDI\n");
  midi_set_mapped_note(36, 2, 36);
  midi_message_t messages[8];
  for(uint32_t i = 0;;) {
    uint32_t received = midi_get_available_messages(messages, 8);
    if(received > 0) {
      printf("Got %d messages\n", received);
    }
    if(pio_display_can_wait_without_blocking()) {
      pio_display_wait_for_finish_blocking();
      end = get_absolute_time();
      i++;
      us += absolute_time_diff_us(start, end);
      start = get_absolute_time();
      pio_display_update_and_flip();
      sdhi_update_displays(values, sdhi);
    }
    sdhi_update_values(values, sdhi);
  }
}
