#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ymf262_synth.h"
#include "ymf262.h"

enum groups {
  SELECTION
};
static const sdhi_group_t const groups[] = {
  {
    .id = SELECTION,
    .title = "Global"
  }
};

enum controls {
  NONE = -1,
  CONNECTION,
  CTRL_TL_1,
  CTRL_TL_2,
  CTRL_TL_3,
  CTRL_TL_4,
  CONTROLS
};

static const shdi_control_type_enumeration_value_t connection_values[] = {
  { .name = "1>2",     .value = FM },
  { .name = "1+2",     .value = AM },
  { .name = "1>2>3>4", .value = FMFM },
  { .name = "1>2+3>4", .value = FM_FM },
  { .name = "1+2>3>4", .value = AM_FM },
  { .name = "1+2>3+4", .value = AM_FM_AM }
};

static const sdhi_control_t const controls[] = {
  {
    .id = CONNECTION,
    .title = "Connection",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = connection_values,
      .size = sizeof(connection_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = AM
    }
  },
  {
    .id = CTRL_TL_1,
    .title = "Level1",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 31,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_TL_2,
    .title = "Level2",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 31,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_TL_3,
    .title = "Level3",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 31,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_TL_4,
    .title = "Level4",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 31,
      .middle = 0,
      .initial = 0
    }
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const uint32_t groups_size = sizeof(groups) / sizeof(sdhi_group_t);
static const sdhi_panel_t const panels[] = {
  {
    "main",
    NULL,
    {
      CTRL_TL_1, CTRL_TL_2, NONE,
      CTRL_TL_3, CTRL_TL_4, NONE,
      CONNECTION, NONE
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

static int32_t values[CONTROLS];

static action_t actions[] = {
  // Initial actions set up six midi slots (0-5) responding to MIDI on channel 0
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
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 2,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 3,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 4,
        .type = PARAMETER_VALUE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_SLOT,
    .configuration.slot = {
      .slot = {
        .parameter.value = 5,
        .type = PARAMETER_VALUE
      }
    }
  },
  // Set up the six 4 OP channels (0 - 5) to accept value changes from slots 0 - 5
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
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 2,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 2,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 2,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 3,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 3,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 3,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 4,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 4,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 4,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_SLOT_STATE,
    .configuration.ymf262_slot_state = {
      .slot = {
        .parameter.value = 5,
        .type = PARAMETER_VALUE
      },
      .state = {
        .parameter.note = {
          .slot = 5,
          .parameter = PARAMETER_MIDI_NOTE_STATE
        },
        .type = PARAMETER_MIDI_NOTE
      },
      .note = {
        .parameter.note = {
          .slot = 5,
          .parameter = PARAMETER_MIDI_NOTE_VALUE
        },
        .type = PARAMETER_MIDI_NOTE
      }
    }
  },
  // Connection
  {
    .channel = 0,
    .type = ACTION_YMF262_CONNECTION,
    .configuration.ymf262_connection = {
      .connection = {
        .parameter.control = {
          .id = CONNECTION,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // Total level
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = TL_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TL_1,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = TL_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TL_2,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = TL_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TL_3,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = TL_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TL_4,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
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
