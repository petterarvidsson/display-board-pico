#pragma once

#include <vector>
#include <mapbox/variant.hpp>
#include <span.hpp>
#include <stdint.h>
#include "sdhi.hpp"
#include "midi.h"
#include "ymf262.h"

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

  struct parameter_control_t {
    int16_t id;
    int32_t offset;
    parameter_control_t(const int16_t id, const int32_t offset) : id(id), offset(offset) {}
    parameter_control_t(const int16_t id) : id(id), offset(0) {}
  };

  struct parameter_midi_note_t {
    uint8_t slot;
    parameter_midi_note_parameter_t parameter;
    parameter_midi_note_t(const uint8_t slot, const parameter_midi_note_parameter_t parameter) : slot(slot), parameter(parameter) {}
  };

  typedef mapbox::util::variant<int, parameter_control_t, parameter_midi_note_t> parameter_t;

  struct trigger_button_t {
    int16_t id;
    trigger_button_t(const int16_t id) : id(id) {}
  };

  struct midi_controller_t {
    uint8_t channel;
    parameter_t number;
    parameter_t value;
    midi_controller_t(const uint8_t channel, const parameter_t number, parameter_t value) : channel(channel), number(number), value(value) {}
  };

  struct midi_bank_change_t {
    uint8_t channel;
    parameter_t value;
    midi_bank_change_t(const uint8_t channel, const parameter_t value) : channel(channel), value(value) {}
  };

  struct midi_rpn_t {
    uint8_t channel;
    parameter_t msb;
    parameter_t lsb;
    parameter_t value;
    midi_rpn_t(const uint8_t channel, const parameter_t msb, const parameter_t lsb, const parameter_t value) : channel(channel), msb(msb), lsb(lsb), value(value) {}
  };

  struct midi_nrpn_t {
    uint8_t channel;
    parameter_t msb;
    parameter_t lsb;
    parameter_t value;
    midi_nrpn_t(const uint8_t channel, const parameter_t msb, const parameter_t lsb, const parameter_t value) : channel(channel), msb(msb), lsb(lsb), value(value) {}
  };

  struct midi_mapping_t {
    uint8_t channel;
    parameter_t note;
    parameter_t value;
    midi_mapping_t(const uint8_t channel, const parameter_t note, const parameter_t value) : channel(channel), note(note), value(value) {}
  };

  struct slot_t {
    uint8_t channel;
    parameter_t slot;
    slot_t(const uint8_t channel, const parameter_t slot) : channel(channel), slot(slot) {}
  };

  struct xg_parameter_change_1_t {
    uint8_t channel;
    parameter_t parameter;
    parameter_t value;
    xg_parameter_change_1_t(const uint8_t channel, const parameter_t parameter, const parameter_t value) : channel(channel), parameter(parameter), value(value) {}
  };

  struct ymf262_slot_state_t {
    parameter_t slot;
    parameter_t state;
    parameter_t note;
    ymf262_slot_state_t(const parameter_t slot, const parameter_t state, const parameter_t note) : slot(slot), state(state), note(note) {}
  };

  struct action_ymf262_parameter_t {
    parameter_t parameter;
    parameter_t value;
    action_ymf262_parameter_t(const parameter_t parameter, const parameter_t value) : parameter(parameter), value(value) {}
    action_ymf262_parameter_t(const ymf262_parameter_t parameter, const parameter_t value) : parameter((int)parameter), value(value) {}
  };

  struct ymf262_connection_t {
    parameter_t connection;
    ymf262_connection_t(const parameter_t connection) : connection(connection) {}
  };

  struct load_values_t {
    trigger_button_t trigger;
    parameter_t patch;
    uint16_t *ids;
    uint16_t ids_size;
    load_values_t(const trigger_button_t trigger, const parameter_t patch, uint16_t * const ids, const uint16_t ids_size) : trigger(trigger), patch(patch), ids(ids), ids_size(ids_size) {}
  };

  struct save_values_t {
    trigger_button_t trigger;
    parameter_t patch;
    uint16_t *ids;
    uint16_t ids_size;
    save_values_t(const trigger_button_t trigger, const parameter_t patch, uint16_t * const ids, const uint16_t ids_size) : trigger(trigger), patch(patch), ids(ids), ids_size(ids_size) {}
  };

  typedef mapbox::util::variant<midi_controller_t, midi_bank_change_t, midi_rpn_t, midi_nrpn_t, midi_mapping_t, slot_t, xg_parameter_change_1_t, ymf262_slot_state_t, action_ymf262_parameter_t, ymf262_connection_t, load_values_t, save_values_t> action_t;

  void action_init(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
  void action_update(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
};
