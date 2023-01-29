#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pio_display.h"
#include "i2c_controller.h"
#include "sdhi.h"
#include "midi.h"

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
      .initial = 10
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

typedef enum {
  ACTION_CONTROLLER,
  ACTION_BANK_CHANGE,
  ACTION_NRPN,
  ACTION_MAPPING,
  ACTION_XG_PARAMETER_CHANGE_1
} action_type_t;

typedef struct {
  const uint8_t number;
} action_controller_configuration_t;

typedef struct {
  const uint8_t msb;
  const uint8_t lsb;
} action_rpn_configuration_t;

typedef struct {
  uint8_t note;
} action_mapping_configuration_t;

typedef struct {
  const uint8_t parameter;
} action_xg_parameter_change_1_configuration_t;

typedef union {
  const action_controller_configuration_t controller;
  const action_rpn_configuration_t rpn;
  action_mapping_configuration_t mapping;
  const action_xg_parameter_change_1_configuration_t xg_parameter_change;
} action_configuration_t;

typedef struct {
  int16_t controller_id;
  const int32_t offset;
  uint8_t channel;
  const midi_message_type_t type;
  action_configuration_t configuration;
} action_t;

static action_t actions_template[] = {
  {
    .controller_id = NONE,
    .offset = 0,
    .channel = 0,
    .type = ACTION_XG_PARAMETER_CHANGE_1,
    .configuration.xg_parameter_change = {
      .parameter = 0x07
    }
  },
  {
    .controller_id = DRUM_TYPE,
    .offset = 0,
    .channel = 0,
    .type = ACTION_MAPPING
  },
  {
    .controller_id = DRUM_SOUND,
    .offset = 0,
    .channel = 0,
    .type = ACTION_BANK_CHANGE
  },
  {
    .controller_id = VOLUME,
    .offset = 0,
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = 7
    }
  },
  {
    .controller_id = ATTACK,
    .offset = 64,
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = 73
    }
  },
  {
    .controller_id = DECAY,
    .offset = 64,
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = 0x01,
      .lsb = 0x64
    }
  },
  {
    .controller_id = RELEASE,
    .offset = 64,
    .channel = 0,
    .type = ACTION_CONTROLLER,
    .configuration.controller = {
      .number = 72
    }
  },
  {
    .controller_id = LPF_CUTOFF,
    .offset = 64,
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = 0x01,
      .lsb = 0x20
    }
  },
  {
    .controller_id = LPF_RESONANCE,
    .offset = 64,
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = 0x01,
      .lsb = 0x21
    }
  },
  {
    .controller_id = HPF_CUTOFF,
    .offset = 64,
    .channel = 0,
    .type = ACTION_NRPN,
    .configuration.rpn = {
      .msb = 0x01,
      .lsb = 0x24
    }
  }
};
static uint8_t actions_template_size = sizeof(actions_template) / sizeof(action_t);
static action_t actions[(sizeof(actions_template) / sizeof(action_t)) * NUMBER_OF_DRUMS];
static uint8_t actions_size = sizeof(actions) / sizeof(action_t);

void copy_actions() {
  for(uint8_t i = 0; i < NUMBER_OF_DRUMS; i++) {
    action_t * current_actions = actions + actions_template_size * i;

    memcpy(current_actions, actions_template, sizeof(actions_template));
    current_actions[1].configuration.mapping.note = drums[i].note;
    for(uint8_t j = 0; j < actions_template_size; j++) {
      current_actions[j].controller_id = controls_template_size * i + current_actions[j].controller_id;
      current_actions[j].channel = i;
    }
  }
}

static int32_t computed_values[sizeof(actions) / sizeof(action_t)];
static int32_t sent_values[sizeof(actions) / sizeof(action_t)];
static uint8_t current_action = 0;

static midi_message_t action_messages[8];

uint8_t execute_action_controller(const action_controller_configuration_t configuration, const uint8_t channel, const int32_t value, midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = configuration.number & 0x7F,
      .value = value & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}


uint8_t execute_action_rpn(const action_rpn_configuration_t configuration, const uint8_t channel, const int32_t value, midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = channel & 0x7F,
      .msb = configuration.msb & 0x7F,
      .lsb = configuration.lsb & 0x7F,
      .value = value & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

uint8_t execute_action_xg_parameter_change_1(const action_xg_parameter_change_1_configuration_t configuration, const uint8_t channel, const int32_t value, midi_message_t * const to_send) {
  uint8_t data[MIDI_EXCLUSIVE_MAX_LENGTH];

  data[0] = 0x08;
  data[1] = channel & 0x7F;
  data[2] = configuration.parameter & 0x7F;
  data[3] = value & 0x7F;

  midi_message_t message = {
    .type = MIDI_EXCLUSIVE_MESSAGE,
    .value.exclusive = {
      .channel = channel & 0x7F,
      .manufacturer_id = 0x43,
      .data_size = 3
    }
  };
  memcpy(message.value.exclusive.data, data, message.value.exclusive.data_size);

  to_send[0] = message;
  return 1;
}

uint8_t execute_action_bank_change(const uint8_t channel, const int32_t value, midi_message_t * const to_send) {
  const midi_message_t c1 = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = 0,
      .value = 127
    }
  };
  const midi_message_t c2 = {
      .type = MIDI_CONTROLLER_MESSAGE,
      .value.controller = {
        .channel = channel & 0x7F,
        .number = 32,
        .value = 0
      }
  };
  const midi_message_t p = {
    .type = MIDI_PROGRAM_CHANGE_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = value & 0x7F
    }
  };
  to_send[0] = c1;
  to_send[1] = c2;
  to_send[2] = p;
  return 3;
}

void execute_action_mapping(const action_mapping_configuration_t configuration, const uint8_t channel, const int32_t value) {
  midi_set_mapped_note(configuration.note, channel & 0x7F, value & 0x7F);
}

bool execute_action(const action_t action, int32_t value) {
  uint8_t messages = 0;
  switch(action.type) {
  case ACTION_CONTROLLER:
    messages = execute_action_controller(action.configuration.controller, action.channel, value, action_messages);
    break;
  case ACTION_NRPN:
    messages = execute_action_rpn(action.configuration.rpn, action.channel, value, action_messages);
    break;
  case ACTION_BANK_CHANGE:
    messages = execute_action_bank_change(action.channel, value, action_messages);
    break;
  case ACTION_MAPPING:
    execute_action_mapping(action.configuration.mapping, action.channel, value);
    messages = 0;
    break;
  }
  if(messages < midi_can_send_messages()) {
    midi_send_messages(action_messages, messages);
    return true;
  } else {
    return false;
  }
}

static void execute_actions() {
  uint8_t last_action;
  if(current_action == 0) {
    last_action = actions_size - 1;
  } else {
    last_action = current_action - 1;
  }

  for(; current_action != last_action; current_action = (current_action + 1) % actions_size) {
    if(sent_values[current_action] != computed_values[current_action] && !execute_action(actions[current_action], computed_values[current_action])) {
      break;
    } else {
      sent_values[current_action] = computed_values[current_action];
    }
  }
}

static void update_computed_values() {
  for(uint8_t i = 0; i < actions_size; i++) {
    if(actions[i].controller_id != NONE) {
      switch(sdhi_type(actions[i].controller_id, sdhi)) {
      case SDHI_CONTROL_TYPE_INTEGER:
        computed_values[i] = sdhi_integer(actions[i].controller_id, values, sdhi) + actions[i].offset;
        break;
      case SDHI_CONTROL_TYPE_ENUMERATION:
        computed_values[i] = sdhi_enumeration(actions[i].controller_id, values, sdhi)  + actions[i].offset;
        break;
      case SDHI_CONTROL_TYPE_REAL:
        break;
      }
    }
  }
}

int main() {
  stdio_init_all();
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
  update_computed_values();
  for(uint8_t i = 0; i < NUMBER_OF_DRUMS; i++) {
    computed_values[actions_template_size * i] = 0x01;
  }
  for(uint8_t i; i < actions_size; i++) {
    while(!execute_action(actions[i], computed_values[i])) {
      sleep_ms(10);
    }
    sent_values[i] = computed_values[i];
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
      update_computed_values();
      execute_actions();
    }
  }
}
