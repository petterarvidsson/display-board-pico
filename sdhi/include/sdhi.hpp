#pragma once
#include <pico/stdlib.h>
#include <i2c_controller.h>
#include <span.hpp>
#include "display_list.hpp"

namespace sdhi {
  struct sdhi_control_description_t {
    uint16_t id;
    const char * const title;
    const int16_t group;
  };

  struct sdhi_control_type_integer_t : sdhi_control_description_t {
    const int32_t min;
    const int32_t max;
    const int32_t initial;
    const int32_t middle;
  };

  struct sdhi_control_type_real_t : sdhi_control_description_t {
    const float min;
    const float max;
    const float step;
  };

  typedef struct {
    const char * name;
    const int32_t value;
  } shdi_control_type_enumeration_value_t;

  struct sdhi_control_type_enumeration_t : sdhi_control_description_t {
    const tcb::span<const shdi_control_type_enumeration_value_t> values;
    uint32_t initial;
  };

  struct shdi_control_type_visual_enumeration_value_t {
    const tcb::span<display_list::Item> display_list;
    const int32_t value;
    shdi_control_type_visual_enumeration_value_t(const tcb::span<display_list::Item> display_list, const int32_t value) : display_list(display_list), value(value) {}
  };

  struct sdhi_control_type_visual_enumeration_t : sdhi_control_description_t {
    const tcb::span<const shdi_control_type_visual_enumeration_value_t> values;
    uint32_t initial;
    uint8_t width;
  };

  typedef mapbox::util::variant<sdhi_control_type_integer_t, sdhi_control_type_real_t, sdhi_control_type_enumeration_t, sdhi_control_type_visual_enumeration_t> sdhi_control_t;

  // First pointer is values array, second pointer is sdhi_t,
  // but can not be passed as such, due to recursiveness of type
  typedef tcb::span<display_list::Item> (*display_list_generator_t)(const int32_t * const, const void * const);

  typedef struct {
    const int16_t id;
    const char * const title;
  } sdhi_group_t;

  typedef struct {
    const char * title;
    const char * subtitle;
    std::array<int32_t, 8> controls;
    const uint8_t generated_display;
    const display_list_generator_t display_list_generator;
  } sdhi_panel_t;

  typedef struct {
    const tcb::span<sdhi_control_t> controls;
    const tcb::span<sdhi_group_t> groups;
    const tcb::span<sdhi_panel_t> panels;
    const char * const panel_selector_title;
  } sdhi_t;
  const sdhi_control_description_t sdhi_description(const sdhi_control_t control);
  const sdhi_control_t * const find_control(const int16_t id, const sdhi_t sdhi);
  void sdhi_init(const sdhi_t sdhi);
  void sdhi_init_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi);
  bool sdhi_update_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi);
  void sdhi_update_displays(const int32_t * const values, const sdhi_t sdhi);
  int32_t sdhi_integer(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
  float sdhi_real(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
  int32_t sdhi_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
  int32_t sdhi_visual_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
}
