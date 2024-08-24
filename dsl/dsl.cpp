#include "dsl.hpp"

namespace dsl {
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial, const int32_t middle) {
    return sdhi::sdhi_control_type_integer_t(id, title, group, min, max, initial, middle);
  }

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial) {
    return sdhi::sdhi_control_type_integer_t(id, title, group, min, max, initial, 0);
  }

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max) {
    return sdhi::sdhi_control_type_integer_t(id, title, group, min, max, 0, 0);
  }

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const float min, const float max, const float step) {
    return sdhi::sdhi_control_type_real_t(id, title, group, min, max, step);
  }

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumValue> values, const uint32_t initial) {
    return sdhi::sdhi_control_type_enumeration_t(id, title, group, values, initial);
  }

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumVisual> values, const uint32_t initial, const uint8_t width) {
    return sdhi::sdhi_control_type_visual_enumeration_t(id, title, group, values, initial, width);
  }

  action::Parameter Parameter(const int16_t id, const int32_t offset) {
    return action::ParameterControl(id, offset);
  }

  action::Parameter Parameter(const int16_t id) {
    return action::ParameterControl(id);
  }

  action::Parameter Midi(const uint8_t slot, const action::MidiNoteType type) {
    return action::ParameterMidiNote(slot, type);
  }
}
