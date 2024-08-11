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
static sdhi_group_t groups[] = {
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

static sdhi_control_t controls[] = {
  sdhi_control_type_integer_t(LOAD, "Load from", PATCH, 0, 127),
  sdhi_control_type_integer_t(SAVE, "Save to", PATCH, 0, 127),
  sdhi_control_type_enumeration_t(CONNECTION, "Connection", SINGLE, connection_values, AM),
  sdhi_control_type_integer_t(FEEDBACK, "Feedback", SINGLE, 0, 7),
  sdhi_control_type_enumeration_t(TREMOLO_DEPTH, "Tremolo depth", EFFECT, tremolo_values, 0),
  sdhi_control_type_enumeration_t(VIBRATO_DEPTH, "Vibrato depth", EFFECT, vibrato_values, 0),
  sdhi_control_type_enumeration_t(OCTAVE_SPLIT, "Octave Split", SINGLE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_TREM_1, "Tremolo", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_TREM_2, "Tremolo", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_TREM_3, "Tremolo", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_TREM_4, "Tremolo", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_VIB_1, "Vibrato", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_VIB_2, "Vibrato", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_VIB_3, "Vibrato", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_VIB_4, "Vibrato", EFFECT, on_off_values, 0),
  sdhi_control_type_enumeration_t(CTRL_EGT_1, "EG Type", SINGLE, egt_values, 0),
  sdhi_control_type_enumeration_t(CTRL_EGT_2, "EG Type", SINGLE, egt_values, 0),
  sdhi_control_type_enumeration_t(CTRL_EGT_3, "EG Type", SINGLE, egt_values, 0),
  sdhi_control_type_enumeration_t(CTRL_EGT_4, "EG Type", SINGLE, egt_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSR_1, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSR_2, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSR_3, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSR_4, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSL_1, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSL_2, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSL_3, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_KSL_4, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  sdhi_control_type_enumeration_t(CTRL_MULT_1, "Multiplier", SINGLE, multiplier_values, 0),
  sdhi_control_type_enumeration_t(CTRL_MULT_2, "Multiplier", SINGLE, multiplier_values, 0),
  sdhi_control_type_enumeration_t(CTRL_MULT_3, "Multiplier", SINGLE, multiplier_values, 0),
  sdhi_control_type_enumeration_t(CTRL_MULT_4, "Multiplier", SINGLE, multiplier_values, 0),
  sdhi_control_type_integer_t(CTRL_TL_1, "Level", SINGLE, 0, 31),
  sdhi_control_type_integer_t(CTRL_TL_2, "Level", SINGLE, 0, 31),
  sdhi_control_type_integer_t(CTRL_TL_3, "Level", SINGLE, 0, 31),
  sdhi_control_type_integer_t(CTRL_TL_4, "Level", SINGLE, 0, 31),
  sdhi_control_type_integer_t(CTRL_AR_1, "Attack", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_AR_2, "Attack", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_AR_3, "Attack", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_AR_4, "Attack", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_DR_1, "Decay", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_DR_2, "Decay", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_DR_3, "Decay", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_DR_4, "Decay", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_SL_1, "Sustain", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_SL_2, "Sustain", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_SL_3, "Sustain", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_SL_4, "Sustain", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_RR_1, "Release", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_RR_2, "Release", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_RR_3, "Release", ADSR, 0, 15),
  sdhi_control_type_integer_t(CTRL_RR_4, "Release", ADSR, 0, 15),
  sdhi_control_type_visual_enumeration_t(CTRL_WS_1, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  sdhi_control_type_visual_enumeration_t(CTRL_WS_2, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  sdhi_control_type_visual_enumeration_t(CTRL_WS_3, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  sdhi_control_type_visual_enumeration_t(CTRL_WS_4, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
};

static int32_t values[CONTROLS];
static i2c_controller_button_t buttons[CONTROLS];

static action_t actions[] = {
  // Initial actions set up six midi slots (0-5) responding to MIDI on channel 0
  slot_t(0, 0),
  slot_t(0, 1),
  slot_t(0, 2),
  slot_t(0, 3),
  slot_t(0, 4),
  slot_t(0, 5),
  // Set up the six 4 OP channels (0 - 5) to accept value changes from slots 0 - 5
  ymf262_slot_state_t(0, parameter_midi_note_t(0, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(0, PARAMETER_MIDI_NOTE_VALUE)),
  ymf262_slot_state_t(1, parameter_midi_note_t(1, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(1, PARAMETER_MIDI_NOTE_VALUE)),
  ymf262_slot_state_t(2, parameter_midi_note_t(2, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(2, PARAMETER_MIDI_NOTE_VALUE)),
  ymf262_slot_state_t(3, parameter_midi_note_t(3, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(3, PARAMETER_MIDI_NOTE_VALUE)),
  ymf262_slot_state_t(4, parameter_midi_note_t(4, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(4, PARAMETER_MIDI_NOTE_VALUE)),
  ymf262_slot_state_t(5, parameter_midi_note_t(5, PARAMETER_MIDI_NOTE_STATE), parameter_midi_note_t(5, PARAMETER_MIDI_NOTE_VALUE)),
  // Load
  load_values_t(trigger_button_t(LOAD), parameter_control_t(LOAD), patch_ids, sizeof(patch_ids) / sizeof(uint16_t)),
  // Save
  save_values_t(trigger_button_t(SAVE), parameter_control_t(SAVE), patch_ids, sizeof(patch_ids) / sizeof(uint16_t)),
  // Connection
  ymf262_connection_t(parameter_control_t(CONNECTION)),
  // FEEDBACK
  action_ymf262_parameter_t(FB, parameter_control_t(FEEDBACK)),
  // TREMOLO DEPTH
  action_ymf262_parameter_t(DAM, parameter_control_t(TREMOLO_DEPTH)),
  // VIBRATO DEPTH
  action_ymf262_parameter_t(DVB, parameter_control_t(VIBRATO_DEPTH)),
  // OCTAVE SPLIT
  action_ymf262_parameter_t(NTS, parameter_control_t(OCTAVE_SPLIT)),
  // Tremolo
  action_ymf262_parameter_t(AM_1, parameter_control_t(CTRL_TREM_1)),
  action_ymf262_parameter_t(AM_2, parameter_control_t(CTRL_TREM_2)),
  action_ymf262_parameter_t(AM_3, parameter_control_t(CTRL_TREM_3)),
  action_ymf262_parameter_t(AM_4, parameter_control_t(CTRL_TREM_4)),
  // Vibrato
  action_ymf262_parameter_t(VIB_1, parameter_control_t(CTRL_VIB_1)),
  action_ymf262_parameter_t(VIB_2, parameter_control_t(CTRL_VIB_2)),
  action_ymf262_parameter_t(VIB_3, parameter_control_t(CTRL_VIB_3)),
  action_ymf262_parameter_t(VIB_4, parameter_control_t(CTRL_VIB_4)),
  // EG Type
  action_ymf262_parameter_t(EGT_1, parameter_control_t(CTRL_EGT_1)),
  action_ymf262_parameter_t(EGT_2, parameter_control_t(CTRL_EGT_2)),
  action_ymf262_parameter_t(EGT_3, parameter_control_t(CTRL_EGT_3)),
  action_ymf262_parameter_t(EGT_4, parameter_control_t(CTRL_EGT_4)),
  // KSL
  action_ymf262_parameter_t(KSL_1, parameter_control_t(CTRL_KSL_1)),
  action_ymf262_parameter_t(KSL_2, parameter_control_t(CTRL_KSL_2)),
  action_ymf262_parameter_t(KSL_3, parameter_control_t(CTRL_KSL_3)),
  action_ymf262_parameter_t(KSL_4, parameter_control_t(CTRL_KSL_4)),
  // KSR
  action_ymf262_parameter_t(KSR_1, parameter_control_t(CTRL_KSR_1)),
  action_ymf262_parameter_t(KSR_2, parameter_control_t(CTRL_KSR_2)),
  action_ymf262_parameter_t(KSR_3, parameter_control_t(CTRL_KSR_3)),
  action_ymf262_parameter_t(KSR_4, parameter_control_t(CTRL_KSR_4)),
  // Multiplier
  action_ymf262_parameter_t(MULT_1, parameter_control_t(CTRL_MULT_1)),
  action_ymf262_parameter_t(MULT_2, parameter_control_t(CTRL_MULT_2)),
  action_ymf262_parameter_t(MULT_3, parameter_control_t(CTRL_MULT_3)),
  action_ymf262_parameter_t(MULT_4, parameter_control_t(CTRL_MULT_4)),
  // Total level
  action_ymf262_parameter_t(TL_1, parameter_control_t(CTRL_TL_1)),
  action_ymf262_parameter_t(TL_2, parameter_control_t(CTRL_TL_2)),
  action_ymf262_parameter_t(TL_3, parameter_control_t(CTRL_TL_3)),
  action_ymf262_parameter_t(TL_4, parameter_control_t(CTRL_TL_4)),
  // Attack Rate
  action_ymf262_parameter_t(AR_1, parameter_control_t(CTRL_AR_1)),
  action_ymf262_parameter_t(AR_2, parameter_control_t(CTRL_AR_2)),
  action_ymf262_parameter_t(AR_3, parameter_control_t(CTRL_AR_3)),
  action_ymf262_parameter_t(AR_4, parameter_control_t(CTRL_AR_4)),
  // Decay rate
  action_ymf262_parameter_t(DR_1, parameter_control_t(CTRL_DR_1)),
  action_ymf262_parameter_t(DR_2, parameter_control_t(CTRL_DR_2)),
  action_ymf262_parameter_t(DR_3, parameter_control_t(CTRL_DR_3)),
  action_ymf262_parameter_t(DR_4, parameter_control_t(CTRL_DR_4)),
  // Sustain level
  action_ymf262_parameter_t(SL_1, parameter_control_t(CTRL_SL_1)),
  action_ymf262_parameter_t(SL_2, parameter_control_t(CTRL_SL_2)),
  action_ymf262_parameter_t(SL_3, parameter_control_t(CTRL_SL_3)),
  action_ymf262_parameter_t(SL_4, parameter_control_t(CTRL_SL_4)),
  // Release rate
  action_ymf262_parameter_t(RR_1, parameter_control_t(CTRL_RR_1)),
  action_ymf262_parameter_t(RR_2, parameter_control_t(CTRL_RR_2)),
  action_ymf262_parameter_t(RR_3, parameter_control_t(CTRL_RR_3)),
  action_ymf262_parameter_t(RR_4, parameter_control_t(CTRL_RR_4)),
  // Waveform select
  action_ymf262_parameter_t(WS_1, parameter_control_t(CTRL_WS_1)),
  action_ymf262_parameter_t(WS_2, parameter_control_t(CTRL_WS_2)),
  action_ymf262_parameter_t(WS_3, parameter_control_t(CTRL_WS_3)),
  action_ymf262_parameter_t(WS_4, parameter_control_t(CTRL_WS_4))
};
static const uint32_t actions_size = sizeof(actions) / sizeof(action_t);
static action_value_t action_values[sizeof(actions) / sizeof(action_t)];

#define MIDI_SLOTS_SIZE 6
midi_slot_t midi_slots[MIDI_SLOTS_SIZE];

#define RADIUS 4

static dl::Item items[] = {
  dl::FilledCircle(dl::Point(0, RADIUS), RADIUS),
  dl::Line(dl::Point(0, RADIUS), dl::Point(0, RADIUS + 45), 1),
  dl::FilledCircle(dl::Point(10, RADIUS + 45), RADIUS),
  dl::Line(dl::Point(0, RADIUS + 45), dl::Point(0, 0), 1),
  dl::FilledCircle(dl::Point(0, 0), RADIUS),
  dl::Line(dl::Point(0, 0), dl::Point(0, RADIUS), 1),
  dl::FilledCircle(dl::Point(0, RADIUS), RADIUS),
  dl::Line(dl::Point(0, 31), dl::Point(0, RADIUS), 1),
  dl::FilledCircle(dl::Point(0, RADIUS), RADIUS)
};

auto list = make_span(items);

static span<dl::Item> generator(const int32_t * const values, const void * const sdhi_ptr,
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

  items[0].get_unchecked<dl::FilledCircle>().center.x = offset;
  items[1].get_unchecked<dl::Line>().start.x = offset;
  items[1].get_unchecked<dl::Line>().end.x = attack + offset;
  items[2].get_unchecked<dl::FilledCircle>().center.x = attack + offset;
  items[3].get_unchecked<dl::Line>().start.x = attack + offset;
  items[3].get_unchecked<dl::Line>().end.x = attack + decay + offset;
  items[3].get_unchecked<dl::Line>().end.y = sustain_y + RADIUS;
  items[4].get_unchecked<dl::FilledCircle>().center.x = attack + decay + offset;
  items[4].get_unchecked<dl::FilledCircle>().center.y = sustain_y + RADIUS;
  items[5].get_unchecked<dl::Line>().start.x = attack + decay + offset;
  items[5].get_unchecked<dl::Line>().start.y = sustain_y + RADIUS;
  items[5].get_unchecked<dl::Line>().end.x = attack + decay + sustain + offset;
  items[5].get_unchecked<dl::Line>().end.y = sustain_y + RADIUS;
  items[6].get_unchecked<dl::FilledCircle>().center.x = attack + decay + sustain + offset;
  items[6].get_unchecked<dl::FilledCircle>().center.y = sustain_y + RADIUS;
  items[7].get_unchecked<dl::Line>().start.x = attack + decay + sustain + offset;
  items[7].get_unchecked<dl::Line>().start.y = sustain_y + RADIUS;
  items[7].get_unchecked<dl::Line>().end.x = attack + decay + sustain + release + offset;
  items[8].get_unchecked<dl::FilledCircle>().center.x = attack + decay + sustain + release + offset;
  return list;
}

static span<dl::Item> osc1_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_1, CTRL_DR_1, CTRL_SL_1, CTRL_RR_1, CTRL_EGT_1);
}

static span<dl::Item> osc2_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_2, CTRL_DR_2, CTRL_SL_2, CTRL_RR_2, CTRL_EGT_2);
}

static span<dl::Item> osc3_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_3, CTRL_DR_3, CTRL_SL_3, CTRL_RR_3, CTRL_EGT_3);
}

static span<dl::Item> osc4_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_4, CTRL_DR_4, CTRL_SL_4, CTRL_RR_3, CTRL_EGT_4);
}
static sdhi_panel_t panels[] = {
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
static sdhi_t sdhi_setup = {
  .controls = controls,
  .groups = groups,
  .panels = panels,
  .panel_selector_title = "Panel",
};


Setup ymf262_synth_init() {
  ymf262_init();
  Setup ymf262_synth = {
    .sdhi = sdhi_setup,
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
