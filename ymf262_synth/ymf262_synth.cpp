#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ymf262_synth.hpp"
#include "dsl.hpp"

using namespace setup;
using namespace dsl;
enum groups {
  SINGLE = -1,
  PATCH,
  ADSR,
  EFFECT,
  KEY_SCALE
};
static sdhi::sdhi_group_t groups[] = {
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

static const EnumValue connection_values[] = {
  EnumValue("1>2", FM),
  EnumValue("1+2", AM),
  EnumValue("1>2>3>4", FMFM),
  EnumValue("1>2+3>4", FM_FM),
  EnumValue("1+2>3>4", AM_FM),
  EnumValue("1+2>3+4", AM_FM_AM)
};

static const EnumValue on_off_values[] = {
  EnumValue("off", 0),
  EnumValue("on", 1)
};

static const EnumValue vibrato_values[] = {
  EnumValue("7%", 0),
  EnumValue("14%", 1)
};

static const EnumValue tremolo_values[] = {
  EnumValue("1dB", 0),
  EnumValue("4.8dB", 1)
};

static const EnumValue multiplier_values[] = {
  EnumValue("0.5", 0),
  EnumValue("1", 1),
  EnumValue("2", 2),
  EnumValue("3", 3),
  EnumValue("4", 4),
  EnumValue("5", 5),
  EnumValue("6", 6),
  EnumValue("7", 7),
  EnumValue("8", 8),
  EnumValue("9", 9),
  EnumValue("10", 10),
  EnumValue("12", 12),
  EnumValue("15", 15)
};

static const EnumValue egt_values[] = {
  EnumValue("decay", 0),
  EnumValue("sustained", 1)
};

static const EnumValue low_high_values[] = {
  EnumValue("low", 0),
  EnumValue("high", 1)
};

#define NODE_RADIUS 2
#define FULL_LENGTH 40
#define HALF_LENGTH 20

static Item sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), FULL_LENGTH + 1, 12, 0, 17)
};
static Item half_sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), HALF_LENGTH + 1, 12, 0, 9)
};

static Item double_half_sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), HALF_LENGTH + 1, 12, 0, 9),
  SineSegment(Point(HALF_LENGTH, 12), HALF_LENGTH + 1, 12, 0, 9)
};

static Item double_quarter_sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), (HALF_LENGTH / 2) + 1, 12, 0, 5),
  FilledCircle(Point(HALF_LENGTH / 2, 24), NODE_RADIUS),
  Line(Point(HALF_LENGTH / 2, 24), Point(HALF_LENGTH / 2, 12), 0),
  FilledCircle(Point(HALF_LENGTH / 2, 12), NODE_RADIUS),
  SineSegment(Point(HALF_LENGTH, 12), (HALF_LENGTH / 2) + 1, 12, 0, 5),
  FilledCircle(Point(HALF_LENGTH + HALF_LENGTH / 2, 24), NODE_RADIUS),
  Line(Point(HALF_LENGTH + HALF_LENGTH / 2, 24), Point(HALF_LENGTH + HALF_LENGTH / 2, 12), 0),
  FilledCircle(Point(HALF_LENGTH + HALF_LENGTH / 2, 12), NODE_RADIUS)
};

static Item double_frequency_sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), HALF_LENGTH + 1, 12, 0, 17),
  FilledCircle(Point(HALF_LENGTH / 2, 12), NODE_RADIUS)
};
static Item double_frequency_double_half_sine_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  SineSegment(Point(0, 12), (HALF_LENGTH / 2) + 1, 12, 0, 9),
  FilledCircle(Point(HALF_LENGTH / 2, 12), NODE_RADIUS),
  SineSegment(Point((HALF_LENGTH / 2), 12), (HALF_LENGTH / 2) + 1, 12, 0, 9)
};
static Item square_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  Line(Point(0, 12), Point(0, 22), 0),
  Line(Point(0, 22), Point(HALF_LENGTH, 22), 0),
  Line(Point(HALF_LENGTH, 22), Point(HALF_LENGTH, 2), 0),
  Line(Point(HALF_LENGTH, 2), Point(FULL_LENGTH, 2), 0),
  Line(Point(FULL_LENGTH, 2), Point(FULL_LENGTH, 12), 0)
};

static Item derived_square_waveform_items[] = {
  FilledCircle(Point(0, 12), NODE_RADIUS),
  FilledCircle(Point(HALF_LENGTH, 12), NODE_RADIUS),
  FilledCircle(Point(FULL_LENGTH, 12), NODE_RADIUS),
  Line(Point(0, 12), Point(FULL_LENGTH, 12), 0),
  Line(Point(0, 12), Point(0, 22), 0),
  Line(Point(0, 22), Point(FULL_LENGTH, 2), 0),
  Line(Point(FULL_LENGTH, 2), Point(FULL_LENGTH, 12), 0)
};

static const EnumVisual waveform_values[] = {
  EnumVisual(make_span(sine_waveform_items), 0),
  EnumVisual(make_span(half_sine_waveform_items), 1),
  EnumVisual(make_span(double_half_sine_waveform_items), 2),
  EnumVisual(make_span(double_quarter_sine_waveform_items), 3),
  EnumVisual(make_span(double_frequency_sine_waveform_items), 4),
  EnumVisual(make_span(double_frequency_double_half_sine_waveform_items), 5),
  EnumVisual(make_span(square_waveform_items), 6),
  EnumVisual(make_span(derived_square_waveform_items), 7)
};

static sdhi::sdhi_control_t controls[] = {
  Control(LOAD, "Load from", PATCH, 0, 127),
  Control(SAVE, "Save to", PATCH, 0, 127),
  Control(CONNECTION, "Connection", SINGLE, connection_values, AM),
  Control(FEEDBACK, "Feedback", SINGLE, 0, 7),
  Control(TREMOLO_DEPTH, "Tremolo depth", EFFECT, tremolo_values, 0),
  Control(VIBRATO_DEPTH, "Vibrato depth", EFFECT, vibrato_values, 0),
  Control(OCTAVE_SPLIT, "Octave Split", SINGLE, low_high_values, 0),
  Control(CTRL_TREM_1, "Tremolo", EFFECT, on_off_values, 0),
  Control(CTRL_TREM_2, "Tremolo", EFFECT, on_off_values, 0),
  Control(CTRL_TREM_3, "Tremolo", EFFECT, on_off_values, 0),
  Control(CTRL_TREM_4, "Tremolo", EFFECT, on_off_values, 0),
  Control(CTRL_VIB_1, "Vibrato", EFFECT, on_off_values, 0),
  Control(CTRL_VIB_2, "Vibrato", EFFECT, on_off_values, 0),
  Control(CTRL_VIB_3, "Vibrato", EFFECT, on_off_values, 0),
  Control(CTRL_VIB_4, "Vibrato", EFFECT, on_off_values, 0),
  Control(CTRL_EGT_1, "EG Type", SINGLE, egt_values, 0),
  Control(CTRL_EGT_2, "EG Type", SINGLE, egt_values, 0),
  Control(CTRL_EGT_3, "EG Type", SINGLE, egt_values, 0),
  Control(CTRL_EGT_4, "EG Type", SINGLE, egt_values, 0),
  Control(CTRL_KSR_1, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSR_2, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSR_3, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSR_4, "Key Scale Rate", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSL_1, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSL_2, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSL_3, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  Control(CTRL_KSL_4, "Key Scale Level", KEY_SCALE, low_high_values, 0),
  Control(CTRL_MULT_1, "Multiplier", SINGLE, multiplier_values, 0),
  Control(CTRL_MULT_2, "Multiplier", SINGLE, multiplier_values, 0),
  Control(CTRL_MULT_3, "Multiplier", SINGLE, multiplier_values, 0),
  Control(CTRL_MULT_4, "Multiplier", SINGLE, multiplier_values, 0),
  Control(CTRL_TL_1, "Level", SINGLE, 0, 31),
  Control(CTRL_TL_2, "Level", SINGLE, 0, 31),
  Control(CTRL_TL_3, "Level", SINGLE, 0, 31),
  Control(CTRL_TL_4, "Level", SINGLE, 0, 31),
  Control(CTRL_AR_1, "Attack", ADSR, 0, 15),
  Control(CTRL_AR_2, "Attack", ADSR, 0, 15),
  Control(CTRL_AR_3, "Attack", ADSR, 0, 15),
  Control(CTRL_AR_4, "Attack", ADSR, 0, 15),
  Control(CTRL_DR_1, "Decay", ADSR, 0, 15),
  Control(CTRL_DR_2, "Decay", ADSR, 0, 15),
  Control(CTRL_DR_3, "Decay", ADSR, 0, 15),
  Control(CTRL_DR_4, "Decay", ADSR, 0, 15),
  Control(CTRL_SL_1, "Sustain", ADSR, 0, 15),
  Control(CTRL_SL_2, "Sustain", ADSR, 0, 15),
  Control(CTRL_SL_3, "Sustain", ADSR, 0, 15),
  Control(CTRL_SL_4, "Sustain", ADSR, 0, 15),
  Control(CTRL_RR_1, "Release", ADSR, 0, 15),
  Control(CTRL_RR_2, "Release", ADSR, 0, 15),
  Control(CTRL_RR_3, "Release", ADSR, 0, 15),
  Control(CTRL_RR_4, "Release", ADSR, 0, 15),
  Control(CTRL_WS_1, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  Control(CTRL_WS_2, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  Control(CTRL_WS_3, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
  Control(CTRL_WS_4, "Waveform", SINGLE, waveform_values, 0, FULL_LENGTH),
};

static int32_t values[CONTROLS];
static i2c_controller_button_t buttons[CONTROLS];

static Action actions[] = {
  // Initial actions set up six midi slots (0-5) responding to MIDI on channel 0
  Slot(0, 0),
  Slot(0, 1),
  Slot(0, 2),
  Slot(0, 3),
  Slot(0, 4),
  Slot(0, 5),
  // Set up the six 4 OP channels (0 - 5) to accept value changes from slots 0 - 5
  YMF262Slot(0, Midi(0, STATE), Midi(0, VALUE)),
  YMF262Slot(1, Midi(1, STATE), Midi(1, VALUE)),
  YMF262Slot(2, Midi(2, STATE), Midi(2, VALUE)),
  YMF262Slot(3, Midi(3, STATE), Midi(3, VALUE)),
  YMF262Slot(4, Midi(4, STATE), Midi(4, VALUE)),
  YMF262Slot(5, Midi(5, STATE), Midi(5, VALUE)),
  // Load
  Load(Trigger(LOAD), Parameter(LOAD), patch_ids, sizeof(patch_ids) / sizeof(uint16_t)),
  // Save
  Save(Trigger(SAVE), Parameter(SAVE), patch_ids, sizeof(patch_ids) / sizeof(uint16_t)),
  // Connection
  YMF262Connection(Parameter(CONNECTION)),
  // FEEDBACK
  YMF262Parameter(FB, Parameter(FEEDBACK)),
  // TREMOLO DEPTH
  YMF262Parameter(DAM, Parameter(TREMOLO_DEPTH)),
  // VIBRATO DEPTH
  YMF262Parameter(DVB, Parameter(VIBRATO_DEPTH)),
  // OCTAVE SPLIT
  YMF262Parameter(NTS, Parameter(OCTAVE_SPLIT)),
  // Tremolo
  YMF262Parameter(AM_1, Parameter(CTRL_TREM_1)),
  YMF262Parameter(AM_2, Parameter(CTRL_TREM_2)),
  YMF262Parameter(AM_3, Parameter(CTRL_TREM_3)),
  YMF262Parameter(AM_4, Parameter(CTRL_TREM_4)),
  // Vibrato
  YMF262Parameter(VIB_1, Parameter(CTRL_VIB_1)),
  YMF262Parameter(VIB_2, Parameter(CTRL_VIB_2)),
  YMF262Parameter(VIB_3, Parameter(CTRL_VIB_3)),
  YMF262Parameter(VIB_4, Parameter(CTRL_VIB_4)),
  // EG Type
  YMF262Parameter(EGT_1, Parameter(CTRL_EGT_1)),
  YMF262Parameter(EGT_2, Parameter(CTRL_EGT_2)),
  YMF262Parameter(EGT_3, Parameter(CTRL_EGT_3)),
  YMF262Parameter(EGT_4, Parameter(CTRL_EGT_4)),
  // KSL
  YMF262Parameter(KSL_1, Parameter(CTRL_KSL_1)),
  YMF262Parameter(KSL_2, Parameter(CTRL_KSL_2)),
  YMF262Parameter(KSL_3, Parameter(CTRL_KSL_3)),
  YMF262Parameter(KSL_4, Parameter(CTRL_KSL_4)),
  // KSR
  YMF262Parameter(KSR_1, Parameter(CTRL_KSR_1)),
  YMF262Parameter(KSR_2, Parameter(CTRL_KSR_2)),
  YMF262Parameter(KSR_3, Parameter(CTRL_KSR_3)),
  YMF262Parameter(KSR_4, Parameter(CTRL_KSR_4)),
  // Multiplier
  YMF262Parameter(MULT_1, Parameter(CTRL_MULT_1)),
  YMF262Parameter(MULT_2, Parameter(CTRL_MULT_2)),
  YMF262Parameter(MULT_3, Parameter(CTRL_MULT_3)),
  YMF262Parameter(MULT_4, Parameter(CTRL_MULT_4)),
  // Total level
  YMF262Parameter(TL_1, Parameter(CTRL_TL_1)),
  YMF262Parameter(TL_2, Parameter(CTRL_TL_2)),
  YMF262Parameter(TL_3, Parameter(CTRL_TL_3)),
  YMF262Parameter(TL_4, Parameter(CTRL_TL_4)),
  // Attack Rate
  YMF262Parameter(AR_1, Parameter(CTRL_AR_1)),
  YMF262Parameter(AR_2, Parameter(CTRL_AR_2)),
  YMF262Parameter(AR_3, Parameter(CTRL_AR_3)),
  YMF262Parameter(AR_4, Parameter(CTRL_AR_4)),
  // Decay rate
  YMF262Parameter(DR_1, Parameter(CTRL_DR_1)),
  YMF262Parameter(DR_2, Parameter(CTRL_DR_2)),
  YMF262Parameter(DR_3, Parameter(CTRL_DR_3)),
  YMF262Parameter(DR_4, Parameter(CTRL_DR_4)),
  // Sustain level
  YMF262Parameter(SL_1, Parameter(CTRL_SL_1)),
  YMF262Parameter(SL_2, Parameter(CTRL_SL_2)),
  YMF262Parameter(SL_3, Parameter(CTRL_SL_3)),
  YMF262Parameter(SL_4, Parameter(CTRL_SL_4)),
  // Release rate
  YMF262Parameter(RR_1, Parameter(CTRL_RR_1)),
  YMF262Parameter(RR_2, Parameter(CTRL_RR_2)),
  YMF262Parameter(RR_3, Parameter(CTRL_RR_3)),
  YMF262Parameter(RR_4, Parameter(CTRL_RR_4)),
  // Waveform select
  YMF262Parameter(WS_1, Parameter(CTRL_WS_1)),
  YMF262Parameter(WS_2, Parameter(CTRL_WS_2)),
  YMF262Parameter(WS_3, Parameter(CTRL_WS_3)),
  YMF262Parameter(WS_4, Parameter(CTRL_WS_4))
};
static const uint32_t actions_size = sizeof(actions) / sizeof(Action);
static StoredValue action_values[sizeof(actions) / sizeof(Action)];

#define MIDI_SLOTS_SIZE 6
midi_slot_t midi_slots[MIDI_SLOTS_SIZE];

#define RADIUS 4

static Item items[] = {
  FilledCircle(Point(0, RADIUS), RADIUS),
  Line(Point(0, RADIUS), Point(0, RADIUS + 45), 1),
  FilledCircle(Point(10, RADIUS + 45), RADIUS),
  Line(Point(0, RADIUS + 45), Point(0, 0), 1),
  FilledCircle(Point(0, 0), RADIUS),
  Line(Point(0, 0), Point(0, RADIUS), 1),
  FilledCircle(Point(0, RADIUS), RADIUS),
  Line(Point(0, 31), Point(0, RADIUS), 1),
  FilledCircle(Point(0, RADIUS), RADIUS)
};

auto list = make_span(items);

static span<Item> generator(const int32_t * const values, const void * const sdhi_ptr,
                                const uint16_t attack_id, const uint16_t decay_id,
                                const uint16_t sustain_id, const uint16_t release_id,
                                const uint16_t type_id) {
  const sdhi::sdhi_t sdhi = *((sdhi::sdhi_t*)sdhi_ptr);
  const uint8_t attack = 30 - sdhi::sdhi_integer(attack_id, values, sdhi) * 2;
  const int8_t decay = 30 - sdhi::sdhi_integer(decay_id, values, sdhi) * 2;
  const uint8_t sustain_y = 45 - sdhi::sdhi_integer(sustain_id, values, sdhi) * 3;
  const uint8_t sustain = sdhi::sdhi_integer(type_id, values, sdhi) ? 30 : 0;
  const uint8_t release = 30 - sdhi::sdhi_integer(release_id, values, sdhi) * 2;
  const uint8_t offset = (127 - (attack + decay + sustain + release)) / 2;

  std::get<FilledCircle>(items[0]).center.x = offset;
  std::get<Line>(items[1]).start.x = offset;
  std::get<Line>(items[1]).end.x = attack + offset;
  std::get<FilledCircle>(items[2]).center.x = attack + offset;
  std::get<Line>(items[3]).start.x = attack + offset;
  std::get<Line>(items[3]).end.x = attack + decay + offset;
  std::get<Line>(items[3]).end.y = sustain_y + RADIUS;
  std::get<FilledCircle>(items[4]).center.x = attack + decay + offset;
  std::get<FilledCircle>(items[4]).center.y = sustain_y + RADIUS;
  std::get<Line>(items[5]).start.x = attack + decay + offset;
  std::get<Line>(items[5]).start.y = sustain_y + RADIUS;
  std::get<Line>(items[5]).end.x = attack + decay + sustain + offset;
  std::get<Line>(items[5]).end.y = sustain_y + RADIUS;
  std::get<FilledCircle>(items[6]).center.x = attack + decay + sustain + offset;
  std::get<FilledCircle>(items[6]).center.y = sustain_y + RADIUS;
  std::get<Line>(items[7]).start.x = attack + decay + sustain + offset;
  std::get<Line>(items[7]).start.y = sustain_y + RADIUS;
  std::get<Line>(items[7]).end.x = attack + decay + sustain + release + offset;
  std::get<FilledCircle>(items[8]).center.x = attack + decay + sustain + release + offset;
  return list;
}

static span<Item> osc1_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_1, CTRL_DR_1, CTRL_SL_1, CTRL_RR_1, CTRL_EGT_1);
}

static span<Item> osc2_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_2, CTRL_DR_2, CTRL_SL_2, CTRL_RR_2, CTRL_EGT_2);
}

static span<Item> osc3_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_3, CTRL_DR_3, CTRL_SL_3, CTRL_RR_3, CTRL_EGT_3);
}

static span<Item> osc4_env_generator(const int32_t * const values, const void * const sdhi_ptr) {
  return generator(values, sdhi_ptr, CTRL_AR_4, CTRL_DR_4, CTRL_SL_4, CTRL_RR_3, CTRL_EGT_4);
}
static sdhi::sdhi_panel_t panels[] = {
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
static sdhi::sdhi_t sdhi_setup = {
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
