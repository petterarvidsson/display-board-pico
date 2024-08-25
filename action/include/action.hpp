#pragma once

#include <vector>
#include <variant>
#include <span.hpp>
#include <stdint.h>
#include "sdhi.hpp"
#include "midi.h"
#include "ymf262.h"

namespace action {

  struct Value {
    int32_t v1;
    int32_t v2;
    int32_t v3;
    uint8_t trigger;
  };

  struct StoredValue {
    Value computed;
    Value sent;
  };

  bool operator== (const Value lhs, const Value rhs);

  enum MidiNoteType {
    VALUE,
    VELOCITY,
    STATE
  };

  struct ParameterControl {
    int16_t id;
    int32_t offset;
    constexpr ParameterControl(const int16_t id, const int32_t offset) : id(id), offset(offset) {}
    constexpr ParameterControl(const int16_t id) : id(id), offset(0) {}
  };

  struct ParameterMidiNote {
    uint8_t slot;
    MidiNoteType type;
    constexpr ParameterMidiNote(const uint8_t slot, const MidiNoteType type) : slot(slot), type(type) {}
  };

  typedef std::variant<int, ParameterControl, ParameterMidiNote> Parameter;

  struct Trigger {
    int16_t id;
    constexpr Trigger(const int16_t id) : id(id) {}
  };

  struct MidiCC {
    uint8_t channel;
    Parameter number;
    Parameter value;
    constexpr MidiCC(const uint8_t channel, const Parameter number, const Parameter value) : channel(channel), number(number), value(value) {}
  };

  struct MidiBank {
    uint8_t channel;
    Parameter value;
    constexpr MidiBank(const uint8_t channel, const Parameter value) : channel(channel), value(value) {}
  };

  struct MidiRPN {
    uint8_t channel;
    Parameter msb;
    Parameter lsb;
    Parameter value;
    constexpr MidiRPN(const uint8_t channel, const Parameter msb, const Parameter lsb, const Parameter value) : channel(channel), msb(msb), lsb(lsb), value(value) {}
  };

  struct MidiNRPN {
    uint8_t channel;
    Parameter msb;
    Parameter lsb;
    Parameter value;
    constexpr MidiNRPN(const uint8_t channel, const Parameter msb, const Parameter lsb, const Parameter value) : channel(channel), msb(msb), lsb(lsb), value(value) {}
  };

  struct MidiMapping {
    uint8_t channel;
    Parameter note;
    Parameter value;
    constexpr MidiMapping(const uint8_t channel, const Parameter note, const Parameter value) : channel(channel), note(note), value(value) {}
  };

  struct Slot {
    uint8_t channel;
    Parameter slot;
    constexpr Slot(const uint8_t channel, const Parameter slot) : channel(channel), slot(slot) {}
  };

  struct XGParameter {
    uint8_t channel;
    Parameter parameter;
    Parameter value;
    constexpr XGParameter(const uint8_t channel, const Parameter parameter, const Parameter value) : channel(channel), parameter(parameter), value(value) {}
  };

  struct YMF262Slot {
    Parameter slot;
    Parameter state;
    Parameter note;
    constexpr YMF262Slot(const Parameter slot, const Parameter state, const Parameter note) : slot(slot), state(state), note(note) {}
  };

  struct YMF262Parameter {
    Parameter parameter;
    Parameter value;
    constexpr YMF262Parameter(const Parameter parameter, const Parameter value) : parameter(parameter), value(value) {}
    constexpr YMF262Parameter(const ymf262_parameter_t parameter, const Parameter value) : parameter((int)parameter), value(value) {}
  };

  struct YMF262Connection {
    Parameter connection;
    constexpr YMF262Connection(const Parameter connection) : connection(connection) {}
  };

  struct Load {
    Trigger trigger;
    Parameter patch;
    uint16_t *ids;
    uint16_t ids_size;
    constexpr Load(const Trigger trigger, const Parameter patch, uint16_t * const ids, const uint16_t ids_size) : trigger(trigger), patch(patch), ids(ids), ids_size(ids_size) {}
  };

  struct Save {
    Trigger trigger;
    Parameter patch;
    uint16_t *ids;
    uint16_t ids_size;
    constexpr Save(const Trigger trigger, const Parameter patch, uint16_t * const ids, const uint16_t ids_size) : trigger(trigger), patch(patch), ids(ids), ids_size(ids_size) {}
  };

  typedef std::variant<MidiCC, MidiBank, MidiRPN, MidiNRPN, MidiMapping, Slot, XGParameter, YMF262Slot, YMF262Parameter, YMF262Connection, Load, Save> Action;

  void action_init(const tcb::span<const Action> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, StoredValue * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
  void action_update(const tcb::span<const Action> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, StoredValue * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
};
