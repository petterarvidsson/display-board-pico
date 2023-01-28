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
  RELEASE,
  LPF_CUTOFF,
  LPF_RESONANCE,
  HPF_CUTOFF,
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
  },
  {
    .id = VOLUME,
    .title = "Volume",
    .group = 1,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = 0
    }
  },
  {
    .id = ATTACK,
    .title = "Attack",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  },
  {
    .id = DECAY,
    .title = "Decay",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  },
  {
    .id = RELEASE,
    .title = "Release",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  },
  {
    .id = LPF_CUTOFF,
    .title = "LPF Cutoff",
    .group = 3,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  },
  {
    .id = LPF_RESONANCE,
    .title = "LPF Resonance",
    .group = 3,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  },
  {
    .id = HPF_CUTOFF,
    .title = "HPF Cutoff",
    .group = 4,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .offset = -64
    }
  }

};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t * const groups = NULL;
static const sdhi_panel_t const panels[] = {
  {
    "Sound",
    {
      NONE,    NONE,  DRUM_SOUND,
      ATTACK,  DECAY, DRUM_TYPE,
      RELEASE, VOLUME
    }
  },
  {
    "Filter",
    {
      LPF_CUTOFF, LPF_RESONANCE, HPF_CUTOFF,
      NONE,       NONE,          NONE,
      NONE,       NONE
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
static uint8_t bd_volume = 0;
static uint8_t bd_attack = 0;
static uint8_t bd_decay = 0;
static uint8_t bd_release = 0;
static uint8_t bd_hpf_cutoff = 0;
static uint8_t bd_lpf_cutoff = 0;
static uint8_t bd_lpf_resonance = 0;

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
  midi_set_mapped_note(36, 0, 36);
  midi_message_t messages[8];
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
    if(sdhi_update_values(values, sdhi)) {
      if(bd_drum_type != sdhi_enumeration(DRUM_TYPE, values, sdhi)) {
        bd_drum_type = sdhi_enumeration(DRUM_TYPE, values, sdhi);
        midi_set_mapped_note(36, 0, bd_drum_type);
      }
      if(bd_drum_sound != sdhi_enumeration(DRUM_SOUND, values, sdhi)) {
        bd_drum_sound = sdhi_enumeration(DRUM_SOUND, values, sdhi);
        midi_send_bank_change(0, bd_drum_sound);
      }
      if(bd_volume != sdhi_integer(VOLUME, values, sdhi)) {
        bd_volume = sdhi_integer(VOLUME, values, sdhi);
        midi_send_volume(0, bd_volume);
      }
      if(bd_attack != sdhi_integer(ATTACK, values, sdhi)) {
        bd_attack = sdhi_integer(ATTACK, values, sdhi);
        midi_send_attack(0, bd_attack);
      }
      if(bd_decay != sdhi_integer(DECAY, values, sdhi)) {
        bd_decay = sdhi_integer(DECAY, values, sdhi);
        midi_send_decay(0, bd_decay);
      }
      if(bd_release != sdhi_integer(RELEASE, values, sdhi)) {
        bd_release = sdhi_integer(RELEASE, values, sdhi);
        midi_send_release(0, bd_release);
      }
      if(bd_hpf_cutoff != sdhi_integer(HPF_CUTOFF, values, sdhi)) {
        bd_hpf_cutoff = sdhi_integer(HPF_CUTOFF, values, sdhi);
        midi_send_hpf_cutoff(0, bd_hpf_cutoff);
      }
      if(bd_lpf_cutoff != sdhi_integer(LPF_CUTOFF, values, sdhi)) {
        bd_lpf_cutoff = sdhi_integer(LPF_CUTOFF, values, sdhi);
        midi_send_lpf_cutoff(0, bd_lpf_cutoff);
      }
      if(bd_lpf_resonance != sdhi_integer(LPF_RESONANCE, values, sdhi)) {
        bd_lpf_resonance = sdhi_integer(LPF_RESONANCE, values, sdhi);
        midi_send_lpf_resonance(0, bd_lpf_resonance);
      }
    }
  }
}
