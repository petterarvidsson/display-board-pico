#pragma once

#include <sdhi.hpp>
#include <action.hpp>

namespace dsl {
  using namespace display_list;
  using namespace tcb;
  using sdhi::EnumValue;
  using sdhi::EnumVisual;
  using action::StoredValue;
  using action::action_t;

  using action::Trigger;
  using action::MidiCC;
  using action::MidiBank;
  using action::MidiRPN;
  using action::MidiNRPN;
  using action::MidiMapping;
  using action::Slot;
  using action::XGParameter;
  using action::YMF262Slot;
  using action::YMF262Parameter;
  using action::YMF262Connection;
  using action::Load;
  using action::Save;

  using action::MidiNoteType::VALUE;
  using action::MidiNoteType::STATE;
  using action::MidiNoteType::VELOCITY;

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial, const int32_t middle);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const float min, const float max, const float step);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumValue> values, const uint32_t initial);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumVisual> values, const uint32_t initial, const uint8_t width);
  action::Parameter Parameter(const int16_t id, const int32_t offset);
  action::Parameter Parameter(const int16_t id);
  action::Parameter Midi(const uint8_t slot, const action::MidiNoteType type);
};
