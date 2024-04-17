#pragma once

#include <vector>
#include <mapbox/variant.hpp>
#include <span.hpp>
#include <stdint.h>
#include "sdhi.hpp"
#include "midi.h"

namespace action {

  struct value_t {
    int32_t v1;
    int32_t v2;
    int32_t v3;
    uint8_t trigger;
  };

  struct action_value_t {
    value_t computed;
    value_t sent;
  };

  bool operator== (const value_t lhs, const value_t rhs);

  typedef enum {
    PARAMETER_MIDI_NOTE_VALUE,
    PARAMETER_MIDI_NOTE_VELOCITY,
    PARAMETER_MIDI_NOTE_STATE
  } parameter_midi_note_parameter_t;

  typedef struct {
    int16_t id;
    int32_t offset;
  } parameter_control_t;

  typedef struct {
    uint8_t slot;
    parameter_midi_note_parameter_t parameter;
  } parameter_midi_note_t;

  typedef mapbox::util::variant<parameter_control_t, int32_t, parameter_midi_note_t> parameter_t;

  typedef struct {
    int16_t id;
  } trigger_button_t;

  typedef struct {
    uint8_t channel;
    parameter_t number;
    parameter_t value;
  } midi_controller_t;

  typedef struct {
    uint8_t channel;
    parameter_t value;
  } midi_bank_change_t;

  typedef struct {
    uint8_t channel;
    parameter_t msb;
    parameter_t lsb;
    parameter_t value;
  } midi_rpn_t;

  typedef struct {
    uint8_t channel;
    parameter_t msb;
    parameter_t lsb;
    parameter_t value;
  } midi_nrpn_t;

  typedef struct {
    uint8_t channel;
    parameter_t note;
    parameter_t value;
  } midi_mapping_t;

  typedef struct {
    uint8_t channel;
    parameter_t slot;
  } slot_t;

  typedef struct {
    uint8_t channel;
    parameter_t parameter;
    parameter_t value;
  } xg_parameter_change_1_t;

  typedef struct {
    parameter_t slot;
    parameter_t state;
    parameter_t note;
  } ymf262_slot_state_t;

  typedef struct {
    parameter_t parameter;
    parameter_t value;
  } action_ymf262_parameter_t;

  typedef struct {
    parameter_t connection;
  } ymf262_connection_t;

  typedef struct {
    trigger_button_t trigger;
    parameter_t patch;
    uint16_t *ids;
    uint16_t ids_size;
  } load_values_t;

  typedef struct {
    trigger_button_t trigger;
    parameter_t patch;
    uint16_t *ids;
    uint16_t ids_size;
  } save_values_t;

  typedef mapbox::util::variant<midi_controller_t, midi_bank_change_t, midi_rpn_t, midi_nrpn_t, midi_mapping_t, slot_t, xg_parameter_change_1_t, ymf262_slot_state_t, action_ymf262_parameter_t, ymf262_connection_t, load_values_t, save_values_t> action_t;

  void action_init(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
  void action_update(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
};
