#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"
#include "midi.h"
#include "action.h"
#include "drum.h"

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

int main() {
  stdio_init_all();
  printf("SDHI\n");
  pio_display_init();
  i2c_controller_init();
  setup_t drums = drum_init();

  sdhi_init(drums.sdhi);
  sdhi_init_values(drums.values, drums.sdhi);
  midi_init();
  multicore_launch_core1(real_time);

  pio_display_update_and_flip();
  sdhi_update_displays(drums.values, drums.sdhi);
  action_init(drums.actions, drums.sdhi, drums.values, drums.action_values);

  for(uint32_t i = 0;;) {
    if(pio_display_can_wait_without_blocking()) {
      pio_display_wait_for_finish_blocking();
      pio_display_update_and_flip();
      sdhi_update_displays(drums.values, drums.sdhi);
    }
    if(sdhi_update_values(drums.values, drums.sdhi)) {
      action_update(drums.actions, drums.sdhi, drums.values, drums.action_values);
    }
  }
}
