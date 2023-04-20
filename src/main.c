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
#include "ymf262_synth.h"

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

int main() {
  stdio_init_all();
  printf("SDHI\n");
  setup_t setup = ymf262_synth_init();
  drum_init();
  pio_display_init();
  i2c_controller_init();

  sdhi_init(setup.sdhi);
  sdhi_init_values(setup.values, setup.sdhi);
  midi_init();
  multicore_launch_core1(real_time);

  pio_display_update_and_flip();
  sdhi_update_displays(setup.values, setup.sdhi);
  action_init(setup.actions, setup.sdhi, setup.values, setup.action_values, setup.midi_slots, setup.midi_slots_size);

  for(uint32_t i = 0;;) {
    if(pio_display_can_wait_without_blocking()) {
      pio_display_wait_for_finish_blocking();
      pio_display_update_and_flip();
      sdhi_update_displays(setup.values, setup.sdhi);
    }
    sdhi_update_values(setup.values, setup.sdhi);
    midi_slots_status(setup.midi_slots, setup.midi_slots_size);
    action_update(setup.actions, setup.sdhi, setup.values, setup.action_values, setup.midi_slots, setup.midi_slots_size);
  }
}
