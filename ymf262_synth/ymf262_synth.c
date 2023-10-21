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
  FEEDBACK,
  TREMOLO_DEPTH,
  VIBRATO_DEPTH,
  CTRL_TREM_1,
  CTRL_TREM_2,
  CTRL_TREM_3,
  CTRL_TREM_4,
  CTRL_VIB_1,
  CTRL_VIB_2,
  CTRL_VIB_3,
  CTRL_VIB_4,
  CTRL_EGT_1,
  CTRL_EGT_2,
  CTRL_EGT_3,
  CTRL_EGT_4,
  CTRL_MULT_1,
  CTRL_MULT_2,
  CTRL_MULT_3,
  CTRL_MULT_4,
  CTRL_TL_1,
  CTRL_TL_2,
  CTRL_TL_3,
  CTRL_TL_4,
  CTRL_AR_1,
  CTRL_AR_2,
  CTRL_AR_3,
  CTRL_AR_4,
  CTRL_DR_1,
  CTRL_DR_2,
  CTRL_DR_3,
  CTRL_DR_4,
  CTRL_SL_1,
  CTRL_SL_2,
  CTRL_SL_3,
  CTRL_SL_4,
  CTRL_RR_1,
  CTRL_RR_2,
  CTRL_RR_3,
  CTRL_RR_4,
  CTRL_WS_1,
  CTRL_WS_2,
  CTRL_WS_3,
  CTRL_WS_4,
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

static const shdi_control_type_enumeration_value_t on_off_values[] = {
  { .name = "off", .value = 0 },
  { .name = "on",  .value = 1 }
};

static const shdi_control_type_enumeration_value_t vibrato_values[] = {
  { .name = "7%",  .value = 0 },
  { .name = "14%", .value = 1 }
};

static const shdi_control_type_enumeration_value_t tremolo_values[] = {
  { .name = "1dB",   .value = 0 },
  { .name = "4.8dB", .value = 1 }
};

static const shdi_control_type_enumeration_value_t multiplier_values[] = {
  { .name = "0.5", .value = 0 },
  { .name = "1",  .value = 1 },
  { .name = "2", .value = 2 },
  { .name = "3",  .value = 3 },
  { .name = "4", .value = 4 },
  { .name = "5",  .value = 5 },
  { .name = "6", .value = 6 },
  { .name = "7",  .value = 7 },
  { .name = "8", .value = 8 },
  { .name = "9",  .value = 9 },
  { .name = "10", .value = 10 },
  { .name = "12",  .value = 12 },
  { .name = "15",  .value = 15 }
};

static const shdi_control_type_enumeration_value_t egt_values[] = {
  { .name = "sustained", .value = 0 },
  { .name = "decay", .value = 1 }
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
    .id = FEEDBACK,
    .title = "Feedback",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = TREMOLO_DEPTH,
    .title = "TremDepth",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = tremolo_values,
      .size = sizeof(tremolo_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = VIBRATO_DEPTH,
    .title = "VibDepth",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = vibrato_values,
      .size = sizeof(vibrato_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_1,
    .title = "Tremolo1",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_2,
    .title = "Tremolo2",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_3,
    .title = "Tremolo3",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_4,
    .title = "Tremolo4",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_1,
    .title = "Vibrato1",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_2,
    .title = "Vibrato2",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_3,
    .title = "Vibrato3",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_4,
    .title = "Vibrato4",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_1,
    .title = "EG Type1",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_2,
    .title = "EG Type2",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_3,
    .title = "EG Type3",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_4,
    .title = "EG Type4",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_1,
    .title = "Mult1",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_2,
    .title = "Mult2",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_3,
    .title = "Mult3",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_4,
    .title = "Mult4",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TL_1,
    .title = "Level",
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
    .title = "Level",
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
    .title = "Level",
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
    .title = "Level",
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
    .id = CTRL_AR_1,
    .title = "Attack",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_2,
    .title = "Attack",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_2,
    .title = "Attack",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_3,
    .title = "Attack",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_4,
    .title = "Attack",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_1,
    .title = "Decay",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_2,
    .title = "Decay",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_3,
    .title = "Decay",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_4,
    .title = "Decay",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_1,
    .title = "Sustain",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_2,
    .title = "Sustain",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_3,
    .title = "Sustain",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_4,
    .title = "Sustain",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_1,
    .title = "Release",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_2,
    .title = "Release",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_3,
    .title = "Release",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_4,
    .title = "Release",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_WS_1,
    .title = "Waveform",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_WS_2,
    .title = "Waveform",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_WS_3,
    .title = "Waveform",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_WS_4,
    .title = "Waveform",
    .group = SELECTION,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 7,
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
      NONE, NONE, VIBRATO_DEPTH,
      NONE, NONE, TREMOLO_DEPTH,
      CONNECTION, NONE
    }
  },
  {
    "main2",
    NULL,
    {
      CTRL_VIB_1, CTRL_VIB_2, CTRL_VIB_3,
      CTRL_VIB_4, CTRL_TREM_1, CTRL_TREM_2,
      CTRL_TREM_3, CTRL_TREM_4
    }
  },
  {
    "main3",
    NULL,
    {
      CTRL_MULT_1, CTRL_MULT_2, CTRL_MULT_3,
      CTRL_MULT_4, CTRL_EGT_1, CTRL_EGT_2,
      CTRL_EGT_3, CTRL_EGT_4
    }
  },
  {
    "OSC 1",
    NULL,
    {
      CTRL_AR_1, CTRL_DR_1, CTRL_TL_1,
      CTRL_SL_1, CTRL_RR_1, CTRL_WS_1,
      CTRL_MULT_1, FEEDBACK
    }
  },
  {
    "OSC 2",
    NULL,
    {
      CTRL_AR_2, CTRL_DR_2, CTRL_TL_2,
      CTRL_SL_2, CTRL_RR_2, CTRL_WS_2,
      CTRL_MULT_2, NONE
    }
  },
  {
    "OSC 3",
    NULL,
    {
      CTRL_AR_3, CTRL_DR_3, CTRL_TL_3,
      CTRL_SL_3, CTRL_RR_3, CTRL_WS_3,
      CTRL_MULT_3, NONE
    }
  },
  {
    "OSC 4",
    NULL,
    {
      CTRL_AR_4, CTRL_DR_4, CTRL_TL_4,
      CTRL_SL_4, CTRL_RR_4, CTRL_WS_4,
      CTRL_MULT_4, NONE
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
  // FEEDBACK
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = FB,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = FEEDBACK,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // TREMOLO DEPTH
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = DAM,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = TREMOLO_DEPTH,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // VIBRATO DEPTH
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = DVB,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = VIBRATO_DEPTH,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // Tremolo
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = AM_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TREM_1,
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
        .parameter.value = AM_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TREM_2,
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
        .parameter.value = AM_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TREM_3,
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
        .parameter.value = AM_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_TREM_4,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // Vibrato
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = VIB_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_VIB_1,
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
        .parameter.value = VIB_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_VIB_2,
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
        .parameter.value = VIB_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_VIB_3,
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
        .parameter.value = VIB_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_VIB_4,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // EG Type
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = EGT_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_EGT_1,
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
        .parameter.value =EGT_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_EGT_2,
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
        .parameter.value = EGT_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_EGT_3,
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
        .parameter.value = EGT_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_EGT_4,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      }
    }
  },
  // Multiplier
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = MULT_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_MULT_1,
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
        .parameter.value = MULT_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_MULT_2,
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
        .parameter.value = MULT_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_MULT_3,
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
        .parameter.value = MULT_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_MULT_4,
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
  },
  // Attack Rate
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = AR_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_AR_1,
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
        .parameter.value = AR_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_AR_2,
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
        .parameter.value = AR_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_AR_3,
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
        .parameter.value = AR_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_AR_4,
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
        .parameter.value = DR_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_DR_1,
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
        .parameter.value = DR_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_DR_2,
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
        .parameter.value = DR_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_DR_3,
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
        .parameter.value = DR_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_DR_4,
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
        .parameter.value = SL_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_SL_1,
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
        .parameter.value = SL_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_SL_2,
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
        .parameter.value = SL_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_SL_3,
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
        .parameter.value = SL_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_SL_4,
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
        .parameter.value = RR_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_RR_1,
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
        .parameter.value = RR_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_RR_2,
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
        .parameter.value = RR_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_RR_3,
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
        .parameter.value = RR_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_RR_4,
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
        .parameter.value = WS_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_WS_1,
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
        .parameter.value = WS_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_WS_2,
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
        .parameter.value = WS_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_WS_3,
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
        .parameter.value = WS_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_WS_4,
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
