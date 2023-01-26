#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "stdio.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"
#include "midi.h"

// Drums
// BD BassDrum
// SD SnareDrum
// LT LowTom
// MT MidTom
// HT HighTom
// RS RimShot
// CP ClaP
// CB CowBell
// CY CYmbal
// OH OpenHihat
// CH ClosedHihat

// Panel 1 Sound
// Drum type
// Drum sound
// Volume
// Attack
// Decay
// Sustain
// Release

// Panel 2 Filter & Effects
// Cutoff
// Resonance
// Reverb
// Chorus

enum controls {
  NONE = -1,
  DRUM_TYPE = 0,
  DRUM_SOUND,
  VOLUME,
  ATTACK,
  DECAY,
  SUSTAIN,
  RELEASE,
  CUTOFF,
  RESONANCE,
  REVERB,
  CHORUS,
  CONTROLS
};

static const shdi_control_type_enumeration_value_t kick_values[] = {
  { .name = "Kick",       .value = 36 },
  { .name = "Kick tight", .value = 35 },
  { .name = "Kick soft",  .value = 33 }
};

static const shdi_control_type_enumeration_value_t sound_values[] = {
  { .name = "Standard",   .value = 0 },
  { .name = "Standard 2", .value = 1 },
  { .name = "Dry",        .value = 2 },
  { .name = "Brilliant",  .value = 3 },
  { .name = "Room",       .value = 8 },
  { .name = "Dark room",  .value = 9 },
  { .name = "Rock",       .value = 16 },
  { .name = "Rock 2",     .value = 17 },
  { .name = "Electro",    .value = 24 },
  { .name = "Analog",     .value = 25 },
  { .name = "Analog 2",   .value = 26 },
  { .name = "Dance",      .value = 27 },
  { .name = "Hip Hop",    .value = 28 },
  { .name = "Jungle",     .value = 29 },
  { .name = "Jazz",       .value = 32 },
  { .name = "Jazz 2",     .value = 33 },
  { .name = "Brush",      .value = 40 },
  { .name = "Symphony",   .value = 48 }
};

static const sdhi_control_t const controls[] = {
  {
    .id = DRUM_TYPE,
    .title = "Type",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = kick_values,
      .size = sizeof(kick_values) / sizeof(shdi_control_type_enumeration_value_t)
    }
  },
  {
    .id = DRUM_SOUND,
    .title = "Variation",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = sound_values,
      .size = sizeof(sound_values) / sizeof(shdi_control_type_enumeration_value_t)
    }
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t * const groups = NULL;
static const sdhi_panel_t const panels[] = {
  {
    "Sound",
    {
      DRUM_TYPE, DRUM_SOUND, NONE,
      NONE,  NONE,  NONE,
      NONE,  NONE
    }
  },
  {
    "Filter",
    {
      NONE,  NONE,  NONE,
      NONE,  NONE,  NONE,
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
static int32_t values[CONTROLS];

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

static uint8_t bd_drum_type = 36;
static uint8_t bd_drum_sound = 0;

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
  midi_set_mapped_note(36, 1, 36);
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
    if(sdhi_update_values(values, sdhi)) {
      if(bd_drum_type != sdhi_enumeration(DRUM_TYPE, values, sdhi)) {
        bd_drum_type = sdhi_enumeration(DRUM_TYPE, values, sdhi);
        midi_set_mapped_note(36, 1, bd_drum_type);
      }
      if(bd_drum_sound != sdhi_enumeration(DRUM_SOUND, values, sdhi)) {
        bd_drum_sound = sdhi_enumeration(DRUM_SOUND, values, sdhi);
        midi_send_bank_change(1, bd_drum_sound);
      }
    }
  }
}
