#pragma once

#include <sdhi.hpp>
#include <action.hpp>

namespace dsl {
  using namespace display_list;
  using namespace tcb;
  using namespace action;
  using sdhi::EnumValue;
  using sdhi::EnumVisual;

  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial, const int32_t middle);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max, const int32_t initial);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const int32_t min, const int32_t max);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const float min, const float max, const float step);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumValue> values, const uint32_t initial);
  sdhi::sdhi_control_t Control(const uint16_t id, const char * title, const int16_t group, const tcb::span<const EnumVisual> values, const uint32_t initial, const uint8_t width);
};
