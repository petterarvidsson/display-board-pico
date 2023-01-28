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
      .size = sizeof(kick_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 1
    }
  },
  {
    .id = DRUM_SOUND,
    .title = "Variation",
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = sound_values,
      .size = sizeof(sound_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 10
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
      .middle = 0,
      .initial = 100
    }
  },
  {
    .id = ATTACK,
    .title = "Attack",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = DECAY,
    .title = "Decay",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = RELEASE,
    .title = "Release",
    .group = 2,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = LPF_CUTOFF,
    .title = "LPF Cutoff",
    .group = 3,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = LPF_RESONANCE,
    .title = "LPF Resonance",
    .group = 3,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = HPF_CUTOFF,
    .title = "HPF Cutoff",
    .group = 4,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = -64,
      .max = 63,
      .middle = 0,
      .initial = 0
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
static int32_t sent_values[CONTROLS];

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

typedef struct {
  const uint16_t controller_id;
  const uint8_t (*changed)(midi_message_t * const);
} action_t;

static uint8_t bd_drum_type(midi_message_t * const to_send) {
  midi_set_mapped_note(36, 0, sdhi_enumeration(DRUM_TYPE, values, sdhi));
  return 0;
}

static uint8_t bd_drum_sound(midi_message_t * const to_send) {
  const midi_message_t c1 = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = 0,
      .number = 0,
      .value = 127
    }
  };
  const midi_message_t c2 = {
      .type = MIDI_CONTROLLER_MESSAGE,
      .value.controller = {
        .channel = 0,
        .number = 32,
        .value = 0
      }
  };
  const midi_message_t p = {
    .type = MIDI_PROGRAM_CHANGE_MESSAGE,
    .value.controller = {
      .channel = 0,
      .number = sdhi_enumeration(DRUM_SOUND, values, sdhi)
    }
  };
  to_send[0] = c1;
  to_send[1] = c2;
  to_send[2] = p;
  return 3;
}

static uint8_t bd_volume(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = 0,
      .number = 7,
      .value = sdhi_integer(VOLUME, values, sdhi) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t bd_attack(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = 0,
      .number = 73,
      .value = (sdhi_integer(ATTACK, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t bd_decay(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = 0,
      .msb = 0x01,
      .lsb = 0x64,
      .value = (sdhi_integer(DECAY, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}


static uint8_t bd_release(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = 0,
      .number = 72,
      .value = (sdhi_integer(RELEASE, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t bd_hpf_cutoff(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = 0,
      .msb = 0x01,
      .lsb = 0x24,
      .value = (sdhi_integer(HPF_CUTOFF, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t bd_lpf_cutoff(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = 0,
      .msb = 0x01,
      .lsb = 0x20,
      .value = (sdhi_integer(LPF_CUTOFF, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t bd_lpf_resonance(midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = 0,
      .msb = 0x01,
      .lsb = 0x21,
      .value = (sdhi_integer(LPF_RESONANCE, values, sdhi) + 64) & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}


static action_t actions[] = {
  {
    .controller_id = DRUM_SOUND,
    .changed = &bd_drum_sound
  },
  {
    .controller_id = DRUM_TYPE,
    .changed = &bd_drum_type
  },
  {
    .controller_id = VOLUME,
    .changed = &bd_volume
  },
  {
    .controller_id = ATTACK,
    .changed = &bd_attack
  },
  {
    .controller_id = DECAY,
    .changed = &bd_decay
  },
  {
    .controller_id = RELEASE,
    .changed = &bd_release
  },
  {
    .controller_id = LPF_CUTOFF,
    .changed = &bd_lpf_cutoff
  },
  {
    .controller_id = LPF_RESONANCE,
    .changed = &bd_lpf_resonance
  },
  {
    .controller_id = HPF_CUTOFF,
    .changed = &bd_hpf_cutoff
  },
};
static uint8_t actions_size = sizeof(actions) / sizeof(action_t);
static uint8_t current_action = 0;

static midi_message_t action_messages[8];

bool execute_action(const action_t action) {
  uint8_t messages = action.changed(action_messages);
  if(messages < midi_can_send_messages()) {
    midi_send_messages(action_messages, messages);
    return true;
  } else {
    return false;
  }
}

int main() {
  stdio_init_all();
  pio_display_init();
  i2c_controller_init();
  sdhi_init(sdhi);
  sdhi_init_values(values, sdhi);
  midi_init();
  multicore_launch_core1(real_time);

  absolute_time_t start;
  absolute_time_t end;
  uint32_t us = 0;
  start = get_absolute_time();
  pio_display_update_and_flip();
  sdhi_update_displays(values, sdhi);

  for(uint8_t i; i < actions_size; i++) {
    while(!execute_action(actions[i])) {
      sleep_ms(10);
    }
  }

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
      uint8_t last_action;
      if(current_action == 0) {
        last_action = actions_size - 1;
      } else {
        last_action = current_action - 1;
      }
      for(; current_action != last_action; current_action = (current_action + 1) % actions_size) {
        bool changed = false;
        switch(sdhi_type(actions[current_action].controller_id, sdhi)) {
        case SDHI_CONTROL_TYPE_INTEGER:
          {
            const int32_t value = sdhi_integer(actions[current_action].controller_id, values, sdhi);
            const int32_t sent_value = sdhi_integer(actions[current_action].controller_id, sent_values, sdhi);
            changed = value != sent_value;
            break;
          }
        case SDHI_CONTROL_TYPE_REAL:
          {
            const float value = sdhi_real(actions[current_action].controller_id, values, sdhi);
            const float sent_value = sdhi_real(actions[current_action].controller_id, sent_values, sdhi);
            changed = value != sent_value;
            break;
          }
        case SDHI_CONTROL_TYPE_ENUMERATION:
          {
            const int32_t value = sdhi_enumeration(actions[current_action].controller_id, values, sdhi);
            const int32_t sent_value = sdhi_enumeration(actions[current_action].controller_id, sent_values, sdhi);
            changed = value != sent_value;
            break;
          }
        }
        if(changed && !execute_action(actions[current_action])) {
          break;
        } else {
          sent_values[actions[current_action].controller_id] = values[actions[current_action].controller_id];
        }
      }
    }
  }
}
