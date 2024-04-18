#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ymf262_synth.hpp"
#include "ymf262.h"
using namespace setup;

enum groups {
  SINGLE = -1,
  PATCH,
  ADSR,
  EFFECT,
  KEY_SCALE
};
static const sdhi_group_t groups[] = {
  {
    .id = PATCH,
    .title = "Patch"
  },
  {
    .id = ADSR,
    .title = "ADSR"
  },
  {
    .id = EFFECT,
    .title = "Effect"
  },
  {
    .id = KEY_SCALE,
    .title = "Key scale"
  },
};

enum controls {
  NONE = -1,
  LOAD,
  SAVE,
  CONNECTION,
  FEEDBACK,
  TREMOLO_DEPTH,
  VIBRATO_DEPTH,
  OCTAVE_SPLIT,
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
  CTRL_KSR_1,
  CTRL_KSR_2,
  CTRL_KSR_3,
  CTRL_KSR_4,
  CTRL_KSL_1,
  CTRL_KSL_2,
  CTRL_KSL_3,
  CTRL_KSL_4,
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
uint16_t patch_ids[] = {
  CONNECTION,
  FEEDBACK,
  TREMOLO_DEPTH,
  VIBRATO_DEPTH,
  OCTAVE_SPLIT,
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
  CTRL_KSR_1,
  CTRL_KSR_2,
  CTRL_KSR_3,
  CTRL_KSR_4,
  CTRL_KSL_1,
  CTRL_KSL_2,
  CTRL_KSL_3,
  CTRL_KSL_4,
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
  CTRL_WS_4
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
  { .name = "decay", .value = 0 },
  { .name = "sustained", .value = 1 }
};

static const shdi_control_type_enumeration_value_t low_high_values[] = {
  { .name = "low", .value = 0 },
  { .name = "high", .value = 1 }
};

#define NODE_RADIUS 2
#define FULL_LENGTH 40
#define HALF_LENGTH 20

static dl::Item sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), FULL_LENGTH + 1, 12, 0, 17)
};
static dl::Item half_sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), HALF_LENGTH + 1, 12, 0, 9)
};

static dl::Item double_half_sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), HALF_LENGTH + 1, 12, 0, 9),
  dl::SineSegment(dl::Point(HALF_LENGTH, 12), HALF_LENGTH + 1, 12, 0, 9)
};

static dl::Item double_quarter_sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), (HALF_LENGTH / 2) + 1, 12, 0, 5),
  dl::FilledCircle(dl::Point(HALF_LENGTH / 2, 24), NODE_RADIUS),
  dl::Line(dl::Point(HALF_LENGTH / 2, 24), dl::Point(HALF_LENGTH / 2, 12), 0),
  dl::FilledCircle(dl::Point(HALF_LENGTH / 2, 12), NODE_RADIUS),
  dl::SineSegment(dl::Point((HALF_LENGTH / 2), 12), (HALF_LENGTH / 2) + 1, 12, 0, 5),
  dl::FilledCircle(dl::Point(HALF_LENGTH + HALF_LENGTH / 2, 24), NODE_RADIUS),
  dl::Line(dl::Point(HALF_LENGTH + HALF_LENGTH / 2, 24), dl::Point(HALF_LENGTH + HALF_LENGTH / 2, 12), 0),
  dl::FilledCircle(dl::Point(HALF_LENGTH + HALF_LENGTH / 2, 12), NODE_RADIUS)
};

static dl::Item double_frequency_sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), HALF_LENGTH + 1, 12, 0, 17),
  dl::FilledCircle(dl::Point(HALF_LENGTH / 2, 12), NODE_RADIUS)
};
static dl::Item double_frequency_double_half_sine_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::SineSegment(dl::Point(0, 12), (HALF_LENGTH / 2) + 1, 12, 0, 9),
  dl::FilledCircle(dl::Point(HALF_LENGTH / 2, 12), NODE_RADIUS),
  dl::SineSegment(dl::Point((HALF_LENGTH / 2), 12), (HALF_LENGTH / 2) + 1, 12, 0, 9)
};
static dl::Item square_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::Line(dl::Point(0, 12), dl::Point(0, 22), 0),
  dl::Line(dl::Point(0, 22), dl::Point(HALF_LENGTH, 22), 0),
  dl::Line(dl::Point(HALF_LENGTH, 22), dl::Point(HALF_LENGTH, 2), 0),
  dl::Line(dl::Point(HALF_LENGTH, 2), dl::Point(FULL_LENGTH, 2), 0),
  dl::Line(dl::Point(FULL_LENGTH, 2), dl::Point(FULL_LENGTH, 12), 0)
};

static dl::Item derived_square_waveform_items[] = {
  dl::FilledCircle(dl::Point(0, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(HALF_LENGTH, 12), NODE_RADIUS),
  dl::FilledCircle(dl::Point(FULL_LENGTH, 12), NODE_RADIUS),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 12), 0),
  dl::Line(dl::Point(0, 12), dl::Point(0, 22), 0),
  dl::Line(dl::Point(0, 12), dl::Point(FULL_LENGTH, 0), 0),
  dl::Line(dl::Point(FULL_LENGTH, 2), dl::Point(FULL_LENGTH, 12), 0)
};

static const shdi_control_type_visual_enumeration_value_t waveform_values[] = {
  shdi_control_type_visual_enumeration_value_t(make_span(sine_waveform_items), 0),
  shdi_control_type_visual_enumeration_value_t(make_span(half_sine_waveform_items), 1),
  shdi_control_type_visual_enumeration_value_t(make_span(double_half_sine_waveform_items), 2),
  shdi_control_type_visual_enumeration_value_t(make_span(double_quarter_sine_waveform_items), 3),
  shdi_control_type_visual_enumeration_value_t(make_span(double_frequency_sine_waveform_items), 4),
  shdi_control_type_visual_enumeration_value_t(make_span(double_frequency_double_half_sine_waveform_items), 5),
  shdi_control_type_visual_enumeration_value_t(make_span(square_waveform_items), 6),
  shdi_control_type_visual_enumeration_value_t(make_span(derived_square_waveform_items), 7)
};

static const sdhi_control_t const controls[] = {
  {
    .id = LOAD,
    .title = "Load from",
    .group = PATCH,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .middle = 0,
      .initial = 0
    }
  },
    {
    .id = SAVE,
    .title = "Save to",
    .group = PATCH,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 127,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CONNECTION,
    .title = "Connection",
    .group = SINGLE,
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
    .group = SINGLE,
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
    .title = "Tremolo depth",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = tremolo_values,
      .size = sizeof(tremolo_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = VIBRATO_DEPTH,
    .title = "Vibrato depth",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = vibrato_values,
      .size = sizeof(vibrato_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = OCTAVE_SPLIT,
    .title = "Octave Split",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_1,
    .title = "Tremolo",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_2,
    .title = "Tremolo",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_3,
    .title = "Tremolo",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_TREM_4,
    .title = "Tremolo",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_1,
    .title = "Vibrato",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_2,
    .title = "Vibrato",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_3,
    .title = "Vibrato",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_VIB_4,
    .title = "Vibrato",
    .group = EFFECT,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = on_off_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_1,
    .title = "EG Type",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_2,
    .title = "EG Type",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_3,
    .title = "EG Type",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(on_off_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_EGT_4,
    .title = "EG Type",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = egt_values,
      .size = sizeof(egt_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSR_1,
    .title = "Key Scale Rate",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSR_2,
    .title = "Key Scale Rate",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSR_3,
    .title = "Key Scale Rate",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSR_4,
    .title = "Key Scale Rate",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSL_1,
    .title = "Key Scale Level",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSL_2,
    .title = "Key Scale Level",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSL_3,
    .title = "Key Scale Level",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_KSL_4,
    .title = "Key Scale Level",
    .group = KEY_SCALE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = low_high_values,
      .size = sizeof(low_high_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_1,
    .title = "Multiplier",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_2,
    .title = "Multiplier",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_3,
    .title = "Multplier",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_ENUMERATION,
    .configuration.enumeration = {
      .values = multiplier_values,
      .size = sizeof(multiplier_values) / sizeof(shdi_control_type_enumeration_value_t),
      .initial = 0
    }
  },
  {
    .id = CTRL_MULT_4,
    .title = "Multplier",
    .group = SINGLE,
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
    .group = SINGLE,
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
    .group = SINGLE,
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
    .group = SINGLE,
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
    .group = SINGLE,
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
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_2,
    .title = "Attack",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_2,
    .title = "Attack",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_3,
    .title = "Attack",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_AR_4,
    .title = "Attack",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_1,
    .title = "Decay",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_2,
    .title = "Decay",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_3,
    .title = "Decay",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_DR_4,
    .title = "Decay",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_1,
    .title = "Sustain",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_2,
    .title = "Sustain",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_3,
    .title = "Sustain",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_SL_4,
    .title = "Sustain",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_1,
    .title = "Release",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_2,
    .title = "Release",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_3,
    .title = "Release",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_RR_4,
    .title = "Release",
    .group = ADSR,
    .type = SDHI_CONTROL_TYPE_INTEGER,
    .configuration.integer = {
      .min = 0,
      .max = 15,
      .middle = 0,
      .initial = 0
    }
  },
  {
    .id = CTRL_WS_1,
    .title = "Waveform",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_VISUAL_ENUMERATION,
    .configuration.visual_enumeration = {
      .values = waveform_values,
      .size = sizeof(waveform_values) / sizeof(shdi_control_type_visual_enumeration_value_t),
      .initial = 0,
      .width = FULL_LENGTH
    }
  },
  {
    .id = CTRL_WS_2,
    .title = "Waveform",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_VISUAL_ENUMERATION,
    .configuration.visual_enumeration = {
      .values = waveform_values,
      .size = sizeof(waveform_values) / sizeof(shdi_control_type_visual_enumeration_value_t),
      .initial = 0,
      .width = 32
    }
  },
  {
    .id = CTRL_WS_3,
    .title = "Waveform",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_VISUAL_ENUMERATION,
    .configuration.visual_enumeration = {
      .values = waveform_values,
      .size = sizeof(waveform_values) / sizeof(shdi_control_type_visual_enumeration_value_t),
      .initial = 0,
      .width = 32
    }
  },
  {
    .id = CTRL_WS_4,
    .title = "Waveform",
    .group = SINGLE,
    .type = SDHI_CONTROL_TYPE_VISUAL_ENUMERATION,
    .configuration.visual_enumeration = {
      .values = waveform_values,
      .size = sizeof(waveform_values) / sizeof(shdi_control_type_visual_enumeration_value_t),
      .initial = 0,
      .width = 32
    }
  }
};
static const uint32_t controls_size = sizeof(controls) / sizeof(sdhi_control_t);
static const uint32_t groups_size = sizeof(groups) / sizeof(sdhi_group_t);

static int32_t values[CONTROLS];
static i2c_controller_button_t buttons[CONTROLS];

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
  // Load
  {
    .channel = 0,
    .type = ACTION_LOAD_VALUES,
    .configuration.load_values = {
      .trigger.id = LOAD,
      .patch = {
        .parameter.control = {
          .id = LOAD,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      },
      .ids = patch_ids,
      .ids_size = sizeof(patch_ids) / sizeof(uint16_t)
    }
  },
  // Save
  {
    .channel = 0,
    .type = ACTION_SAVE_VALUES,
    .configuration.load_values = {
      .trigger.id = SAVE,
      .patch = {
        .parameter.control = {
          .id = SAVE,
          .offset = 0
        },
        .type = PARAMETER_CONTROL
      },
      .ids = patch_ids,
      .ids_size = sizeof(patch_ids) / sizeof(uint16_t)
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
  // OCTAVE SPLIT
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = NTS,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = OCTAVE_SPLIT,
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
  {
    .channel = 0,
    .type = ACTION_YMF262_PARAMETER,
    .configuration.ymf262_parameter = {
      .parameter = {
        .parameter.value = KSL_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSL_1,
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
        .parameter.value = KSL_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSL_2,
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
        .parameter.value = KSL_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSL_3,
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
        .parameter.value = KSL_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSL_4,
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
        .parameter.value = KSR_1,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSR_1,
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
        .parameter.value = KSR_2,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSR_2,
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
        .parameter.value = KSR_3,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSR_3,
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
        .parameter.value = KSR_4,
        .type = PARAMETER_VALUE
      },
      .value = {
        .parameter.control = {
          .id = CTRL_KSR_4,
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

#define RADIUS 4

static dl::item items[] = {
  {
    .type = DISPLAY_LIST_FILLED_CIRCLE,
    .configuration.circle = {
      .center = {
        .x = 0,
        .y = RADIUS
      },
      .radius = RADIUS
    }
  },
  {
    .type = DISPLAY_LIST_LINE,
    .configuration.line = {
      .start = {
        .x = 0,
        .y = RADIUS
      },
      .end = {
        .x = 0,
        .y = RADIUS + 45
      },
      .size = 1
    }
  },
  {
    .type = DISPLAY_LIST_FILLED_CIRCLE,
    .configuration.circle = {
      .center = {
        .x = 10,
        .y = RADIUS + 45
      },
      .radius = RADIUS
    }
  },
  {
    .type = DISPLAY_LIST_LINE,
    .configuration.line = {
      .start = {
        .x = 0,
        .y = RADIUS + 45
      },
      .end = {
        .x = 0,
        .y = 0
      },
      .size = 1
    }
  },
  {
    .type = DISPLAY_LIST_FILLED_CIRCLE,
    .configuration.circle = {
      .center = {
        .x = 0,
        .y = 0
      },
      .radius = RADIUS
    }
  },
  {
    .type = DISPLAY_LIST_LINE,
    .configuration.line = {
      .start = {
        .x = 0,
        .y = 0
      },
      .end = {
        .x = 0,
        .y = RADIUS
      },
      .size = 1
    }
  },
  {
    .type = DISPLAY_LIST_FILLED_CIRCLE,
    .configuration.circle = {
      .center = {
        .x = 0,
        .y = RADIUS
      },
      .radius = RADIUS
    }
  },
  {
    .type = DISPLAY_LIST_LINE,
    .configuration.line = {
      .start = {
        .x = 0,
        .y = 31
      },
      .end = {
        .x = 0,
        .y = RADIUS
      },
      .size = 1
    }
  },
  {
    .type = DISPLAY_LIST_FILLED_CIRCLE,
    .configuration.circle = {
      .center = {
        .x = 0,
        .y = RADIUS
      },
      .radius = RADIUS
    }
  }
};
static const uint8_t items_size = sizeof(items) / sizeof(display_list_item_t);

static display_list_t list = {
  .items = items,
  .size = items_size
};

static display_list_t generator(const int32_t * const values, const void * const sdhi_ptr,
                                const uint16_t attack_id, const uint16_t decay_id,
                                const uint16_t sustain_id, const uint16_t release_id,
                                const uint16_t type_id) {
  const sdhi_t sdhi = *((sdhi_t*)sdhi_ptr);
  const uint8_t attack = 30 - sdhi_integer(attack_id, values, sdhi) * 2;
  const int8_t decay = 30 - sdhi_integer(decay_id, values, sdhi) * 2;
  const uint8_t sustain_y = 45 - sdhi_integer(sustain_id, values, sdhi) * 3;
  const uint8_t sustain = sdhi_integer(type_id, values, sdhi) ? 30 : 0;
  const uint8_t release = 30 - sdhi_integer(release_id, values, sdhi) * 2;
  const uint8_t offset = (127 - (attack + decay + sustain + release)) / 2;

  items[0].configuration.circle.center.x = offset;
  items[1].configuration.line.start.x = offset;
  items[1].configuration.line.end.x = attack + offset;
  items[2].configuration.circle.center.x = attack + offset;
  items[3].configuration.line.start.x = attack + offset;
  items[3].configuration.line.end.x = attack + decay + offset;
  items[3].configuration.line.end.y = sustain_y + RADIUS;
  items[4].configuration.circle.center.x = attack + decay + offset;
  items[4].configuration.circle.center.y = sustain_y + RADIUS;
  items[5].configuration.line.start.x = attack + decay + offset;
  items[5].configuration.line.start.y = sustain_y + RADIUS;
  items[5].configuration.line.end.x = attack + decay + sustain + offset;
  items[5].configuration.line.end.y = sustain_y + RADIUS;
  items[6].configuration.circle.center.x = attack + decay + sustain + offset;
  items[6].configuration.circle.center.y = sustain_y + RADIUS;
  items[7].configuration.line.start.x = attack + decay + sustain + offset;
  items[7].configuration.line.start.y = sustain_y + RADIUS;
  items[7].configuration.line.end.x = attack + decay + sustain + release + offset;
  items[8].configuration.circle.center.x = attack + decay + sustain + release + offset;
  return list;
}

static display_list_t osc1_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_1, CTRL_DR_1, CTRL_SL_1, CTRL_RR_1, CTRL_EGT_1);
}

static display_list_t osc2_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_2, CTRL_DR_2, CTRL_SL_2, CTRL_RR_2, CTRL_EGT_2);
}

static display_list_t osc3_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_3, CTRL_DR_3, CTRL_SL_3, CTRL_RR_3, CTRL_EGT_3);
}

static display_list_t osc4_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_4, CTRL_DR_4, CTRL_SL_4, CTRL_RR_3, CTRL_EGT_4);
}
static const sdhi_panel_t const panels[] = {
  {
    "Global",
    NULL,
    {
      CONNECTION, OCTAVE_SPLIT, NONE,
      TREMOLO_DEPTH, VIBRATO_DEPTH, NONE,
      LOAD, SAVE
    },
    0,
    NULL
  },
  {
    "OSC 1 ENV",
    NULL,
    {
      CTRL_AR_1, CTRL_DR_1, CTRL_EGT_1,
      CTRL_SL_1, CTRL_RR_1, NONE,
      CTRL_KSR_1, CTRL_KSL_1
    },
    13,
    &osc1_env_generator
  },
  {
    "OSC 1 Sound",
    NULL,
    {
      CTRL_MULT_1, CTRL_WS_1, CTRL_TL_1,
      FEEDBACK, CTRL_VIB_1, CTRL_TREM_1,
      NONE, NONE
    },
    0,
    NULL
  },
  {
    "OSC 2 ENV",
    NULL,
    {
      CTRL_AR_2, CTRL_DR_2, CTRL_EGT_2,
      CTRL_SL_2, CTRL_RR_2, NONE,
      CTRL_KSR_2, CTRL_KSL_2
    },
    13,
    &osc2_env_generator
  },
  {
    "OSC 2 Sound",
    NULL,
    {
      CTRL_MULT_2, CTRL_WS_2, CTRL_TL_2,
      NONE, CTRL_VIB_2, CTRL_TREM_2,
      NONE, NONE
    },
    0,
    NULL
  },
  {
    "OSC 3 ENV",
    NULL,
    {
      CTRL_AR_3, CTRL_DR_3, CTRL_EGT_3,
      CTRL_SL_3, CTRL_RR_3, NONE,
      CTRL_KSR_3, CTRL_KSL_3
    },
    13,
    &osc3_env_generator
  },
  {
    "OSC 3 Sound",
    NULL,
    {
      CTRL_MULT_3, CTRL_WS_3, CTRL_TL_3,
      NONE, CTRL_VIB_3, CTRL_TREM_3,
      NONE, NONE
    },
    0,
    NULL
  },
  {
    "OSC 4 ENV",
    NULL,
    {
      CTRL_AR_4, CTRL_DR_4, CTRL_EGT_4,
      CTRL_SL_4, CTRL_RR_4, NONE,
      CTRL_KSR_4, CTRL_KSL_4
    },
    13,
    &osc4_env_generator
  },
  {
    "OSC 4 Sound",
    NULL,
    {
      CTRL_MULT_4, CTRL_WS_4, CTRL_TL_4,
      NONE, CTRL_VIB_4, CTRL_TREM_4,
      NONE, NONE
    },
    0,
    NULL
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


Setup ymf262_synth_init() {
  ymf262_init();
  setup_t ymf262_synth = {
    .sdhi = sdhi,
    .values = values,
    .buttons = buttons,
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
