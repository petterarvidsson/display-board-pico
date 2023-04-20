#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ymf262_synth.h"
enum controls {
  NONE = -1
};

static const sdhi_control_t const controls[] = {};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const sdhi_group_t const groups[] = {};
static const uint32_t groups_size = sizeof(groups) / sizeof(sdhi_group_t);
static const sdhi_panel_t const panels[] = {
  {
    "main",
    NULL,
    {
      NONE, NONE, NONE,
      NONE, NONE, NONE,
      NONE, NONE
    }
  }
};
static const uint32_t panels_size = sizeof(panels) / sizeof(sdhi_panel_t);
static sdhi_t sdhi = {
  .controls = controls,
  .controls_size = controls_size,
  .groups = groups,
  .groups_size = groups_size,
  .panel_selector_title = "Panel",
  .panels = panels,
  .panels_size = panels_size
};

static int32_t values[0];

static action_t actions[] = {
  {
    .channel = 0,
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 0,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 0,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 0,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 0,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 1,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 1,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 1,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 1,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  }
};
static const uint32_t actions_size = sizeof(actions) / sizeof(action_t);
static action_value_t action_values[sizeof(actions) / sizeof(action_t)];

#define MIDI_SLOTS_SIZE 6
midi_slot_t midi_slots[MIDI_SLOTS_SIZE];

setup_t ymf262_synth_init() {
  ymf262_init();
  setup_t ymf262_synth = {
    .sdhi = sdhi,
    .values = values,
    .actions = {
      .actions = actions,
      .size = actions_size
    },
    .action_values = action_values,
    .midi_slots = midi_slots,
    .midi_slots_size = MIDI_SLOTS_SIZE
  };
  return ymf262_synth;
}
