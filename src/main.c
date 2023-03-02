#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"
#include "midi.h"
#include "action.h"

#define NUMBER_OF_DRUMS 11

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
  CONTROLS
};

static const shdi_control_type_enumeration_value_t kick_values[] = {
  { .name = "Kick",       .value = 36 },
  { .name = "Kick tight", .value = 35 },
  { .name = "Kick soft",  .value = 33 }
};

static const shdi_control_type_enumeration_value_t snare_values[] = {
  { .name = "Snare", .value = 38 },
  { .name = "Snare tight", .value = 40 },
  { .name = "Snare soft", .value = 31 },
  { .name = "Open rim shot", .value = 34 },
  { .name = "Brush tap", .value = 25 },
  { .name = "Brush swirl", .value = 26 },
  { .name = "Brush slap", .value = 27 },
  { .name = "Brush tap swirl", .value = 28 }
};

static const shdi_control_type_enumeration_value_t cowbell_values[] = {
  { .name = "Metronome bell", .value = 21 },
  { .name = "Cowbell",        .value = 56 },
  { .name = "Tambourine",     .value = 54 },
  { .name = "Jingle bells",   .value = 83 },
  { .name = "Bell tree",      .value = 84 }
};

static const shdi_control_type_enumeration_value_t tom_values[] = {
  { .name = "Floor tom L", .value = 41 },
  { .name = "Floor tom H", .value = 43 },
  { .name = "Low tom",     .value = 45 },
  { .name = "Mid tom L",   .value = 47 },
  { .name = "Mid tom H",   .value = 48 },
  { .name = "High tom",    .value = 50 }
};

static const shdi_control_type_enumeration_value_t cymbal_values[] = {
  { .name = "Crash 1",  .value = 49 },
  { .name = "Crash 2",  .value = 57 },
  { .name = "Splash",   .value = 55 },
  { .name = "Ride 1",   .value = 51 },
  { .name = "Ride 2",   .value = 59 },
  { .name = "Ride cup", .value = 53 },
  { .name = "Chinese",  .value = 52 }
};

static const shdi_control_type_enumeration_value_t open_hihat_values[] = {
  { .name = "Open hi-hat",  .value = 46 }
};

static const shdi_control_type_enumeration_value_t closed_hihat_values[] = {
  { .name = "Closed hi-hat",  .value = 42 }
};

static const shdi_control_type_enumeration_value_t clap_values[] = {
  { .name = "Hand clap", .value = 39 },
  { .name = "Whip slap", .value = 16 },
  { .name = "Finger snap", .value = 19 },
  { .name = "Castanet", .value = 30 },
  { .name = "Sticks", .value = 32 }
};

typedef struct {
  const char * name;
  const uint8_t note;
  const shdi_control_type_enumeration_value_t * values;
  const uint8_t values_size;
  const uint8_t initial;
} drum_t;

drum_t drums[NUMBER_OF_DRUMS] = {
  {
    .name = "Bass drum",
    .note = 36,
    .values = kick_values,
    .values_size = sizeof(kick_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Snare drum",
    .note = 38,
    .values = snare_values,
    .values_size = sizeof(snare_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Low tom",
    .note = 41,
    .values = tom_values,
    .values_size = sizeof(tom_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Mid tom",
    .note = 43,
    .values = tom_values,
    .values_size = sizeof(tom_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 3
  },
  {
    .name = "High tom",
    .note = 45,
    .values = tom_values,
    .values_size = sizeof(tom_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 5
  },
  {
    .name = "Snare rim",
    .note = 40,
    .values = snare_values,
    .values_size = sizeof(snare_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 3
  },
  {
    .name = "Clap",
    .note = 39,
    .values = clap_values,
    .values_size = sizeof(clap_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Cowbell",
    .note = 56,
    .values = cowbell_values,
    .values_size = sizeof(cowbell_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Cymbal",
    .note = 49,
    .values = cymbal_values,
    .values_size = sizeof(cymbal_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Open Hihat",
    .note = 46,
    .values = open_hihat_values,
    .values_size = sizeof(open_hihat_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
  {
    .name = "Closed Hihat",
    .note = 42,
    .values = closed_hihat_values,
    .values_size = sizeof(closed_hihat_values) / sizeof(shdi_control_type_enumeration_value_t),
    .initial = 0
  },
};

static shdi_control_type_enumeration_value_t sound_values[] = {
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

static char type_title[] = "Type";
static char sound_title[] = "Variation";
static char volume_title[] = "Volume";
static char attack_title[] = "Attack";
static char decay_title[] = "Decay";
static char release_title[] = "Release";
static char lpf_cutoff_title[] = "LPF Cutoff";
static char lpf_resonance_title[] = "LPF Resonance";
static char hpf_cutoff_title[] = "HPF Cutoff";

static const sdhi_control_t const controls_template[] = {
  {
    .id = DRUM_TYPE,
    .title = type_title,
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
  },
  {
    .id = DRUM_SOUND,
    .title = sound_title,
    .group = 0,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = sound_values,
      .size = sizeof(sound_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = VOLUME,
    .title = volume_title,
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
    .title = attack_title,
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
    .title = decay_title,
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
    .title = release_title,
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
    .title = lpf_cutoff_title,
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
    .title = lpf_resonance_title,
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
    .title = hpf_cutoff_title,
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
static const uint32_t controls_template_size = sizeof(controls_template) / sizeof(sdhi_control_t);

static sdhi_control_t controls[(sizeof(controls_template) / sizeof(sdhi_control_t)) * NUMBER_OF_DRUMS];
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);

static const sdhi_group_t * const groups = NULL;
static const char panel_sound[] = "Sound";
static const char panel_filter[] = "Filter";

static const sdhi_panel_t const panels_template[] = {
  {
    panel_sound,
    NULL,
    {
      NONE,    NONE,  DRUM_SOUND,
      ATTACK,  DECAY, DRUM_TYPE,
      RELEASE, VOLUME
    }
  },
  {
    panel_filter,
    NULL,
    {
      LPF_CUTOFF, LPF_RESONANCE, HPF_CUTOFF,
      NONE,       NONE,          NONE,
      NONE,       NONE
    }
  }
};
static const uint32_t panels_template_size = sizeof(panels_template) / sizeof(sdhi_panel_t);
static sdhi_panel_t panels[(sizeof(panels_template) / sizeof(sdhi_panel_t)) * NUMBER_OF_DRUMS];
static const uint32_t panels_size = sizeof(panels) / sizeof(sdhi_panel_t);

static void copy() {
  for(uint8_t i = 0; i < NUMBER_OF_DRUMS; i++) {
    sdhi_control_t *current_controls = controls + controls_template_size * i;
    sdhi_panel_t *current_panels = panels + panels_template_size * i;
    memcpy(current_controls, controls_template, sizeof(controls_template));

    current_controls[0].configuration.enumeration.values = drums[i].values;
    current_controls[0].configuration.enumeration.size = drums[i].values_size;
    current_controls[0].configuration.enumeration.initial = drums[i].initial;

    for(uint8_t j = 0; j < controls_template_size; j++) {
      current_controls[j].id = controls_template_size * i + current_controls[j].id;
    }
    memcpy(current_panels, panels_template, sizeof(panels_template));
    for(uint8_t j = 0; j < panels_template_size; j++) {
      current_panels[j].subtitle = drums[i].name;
      for(uint8_t k = 0; k < 8; k++) {
        if(current_panels[j].controls[k] != NONE) {
          current_panels[j].controls[k] = controls_template_size * i + current_panels[j].controls[k];
        }
      }
    }
  }
}

static sdhi_t sdhi = {
  controls,
  controls_size,
  groups,
  0,
  "Panel",
  panels,
  panels_size
};

static int32_t values[CONTROLS * NUMBER_OF_DRUMS];

static void real_time() {
  for(uint32_t i = 0;;i++) {
    i2c_controller_run();
    midi_run();
  }
}

static action_t actions_template[] = {
  {
    .channel = 0,
    .type = ACTION_XG_PARAMETER_CHANGE_1,
    .configuration.xg_parameter_change = {
      .parameter = {
        .parameter.value = 0x07,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.value = 0x01,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_MAPPING,
    .configuration.mapping = {
      .value = {
        .parameter.control = {
          .id = DRUM_TYPE,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_BANK_CHANGE,
    .configuration.bank_change = {
      .value = {
        .parameter.control = {
          .id = DRUM_SOUND,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = {
        .parameter.value = 7,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = VOLUME,
          .offset = 0,
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = {
        .parameter.value = 73,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = ATTACK,
          .offset = 64
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = {
        .parameter.value = 0x01,
        .type = PARAMETER_VALUE
      },
      .lsb = {
        .parameter.value = 0x64,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = DECAY,
          .offset = 64,
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = {
        .parameter.value = 72,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = RELEASE,
          .offset = 64,
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = {
        .parameter.value = 0x01,
        .type = PARAMETER_VALUE
      },
      .lsb = {
        .parameter.value = 0x20,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = LPF_CUTOFF,
          .offset = 64,
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = {
        .parameter.value = 0x01,
        .type = PARAMETER_VALUE
      },
      .lsb = {
        .parameter.value = 0x21,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = LPF_RESONANCE,
          .offset = 64
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = {
        .parameter.value = 0x01,
        .type = PARAMETER_VALUE
      },
      .lsb = {
        .parameter.value = 0x24,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = HPF_CUTOFF,
          .offset = 64,
        },
        .type = PARAMETER_CONTROL
      }
    }
  }
};
static uint8_t actions_template_size = sizeof(actions_template) / sizeof(action_t);
static action_t actions[(sizeof(actions_template) / sizeof(action_t)) * NUMBER_OF_DRUMS];
static uint8_t actions_size = sizeof(actions) / sizeof(action_t);

parameter_t parameter_config(parameter_t parameter, const uint32_t offset) {
  if(parameter.type == PARAMETER_CONTROL) {
    parameter.parameter.control.id = parameter.parameter.control.id + offset;
  }
  return parameter;
}

void copy_actions() {
  for(uint8_t i = 0; i < NUMBER_OF_DRUMS; i++) {
    action_t * current_actions = actions + actions_template_size * i;

    memcpy(current_actions, actions_template, sizeof(actions_template));
    current_actions[1].configuration.mapping.note.parameter.value = drums[i].note;
    current_actions[1].configuration.mapping.note.type = PARAMETER_VALUE;
    for(uint8_t j = 0; j < actions_template_size; j++) {
      switch(current_actions[j].type) {
      case ACTION_CONTROLLER:
        current_actions[j].configuration.controller.number =
          parameter_config(current_actions[j].configuration.controller.number, controls_template_size * i);
        current_actions[j].configuration.controller.value =
          parameter_config(current_actions[j].configuration.controller.value, controls_template_size * i);
        break;
      case ACTION_NRPN:
        current_actions[j].configuration.rpn.msb =
          parameter_config(current_actions[j].configuration.rpn.msb, controls_template_size * i);
        current_actions[j].configuration.rpn.lsb =
          parameter_config(current_actions[j].configuration.rpn.lsb, controls_template_size * i);
        current_actions[j].configuration.rpn.value =
          parameter_config(current_actions[j].configuration.rpn.value, controls_template_size * i);
        break;
      case ACTION_BANK_CHANGE:
        current_actions[j].configuration.bank_change.value =
          parameter_config(current_actions[j].configuration.bank_change.value, controls_template_size * i);
        break;
      case ACTION_MAPPING:
        current_actions[j].configuration.mapping.note =
          parameter_config(current_actions[j].configuration.mapping.note, controls_template_size * i);
        current_actions[j].configuration.mapping.value =
          parameter_config(current_actions[j].configuration.mapping.value, controls_template_size * i);
        break;
      }
      current_actions[j].channel = i;
    }
  }
}

static action_value_t action_values[sizeof(actions) / sizeof(action_t)];

int main() {
  stdio_init_all();
  printf("SDHI\n");
  pio_display_init();
  i2c_controller_init();
  copy();
  copy_actions();
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
  action_init(actions, actions_size, sdhi, values, action_values);

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
      action_update(actions, actions_size, sdhi, values, action_values);
    }
  }
}
