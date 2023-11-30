#pragma once
#include <pico/stdlib.h>
#include <i2c_controller.h>
#include "display_list.h"

typedef enum {
  SDHI_CONTROL_TYPE_INTEGER,
  SDHI_CONTROL_TYPE_REAL,
  SDHI_CONTROL_TYPE_ENUMERATION,
  SDHI_CONTROL_TYPE_VISUAL_ENUMERATION
} sdhi_control_type_t;

typedef struct {
  const int32_t min;
  const int32_t max;
  const int32_t initial;
  const int32_t middle;
} sdhi_control_type_integer_t;

typedef struct {
  const float min;
  const float max;
  const float step;
} sdhi_control_type_real_t;

typedef struct {
  const char const * name;
  const int32_t value;
} shdi_control_type_enumeration_value_t;

typedef struct {
  const shdi_control_type_enumeration_value_t *values;
  uint16_t size;
  uint32_t initial;
} sdhi_control_type_enumeration_t;

typedef struct {
  const display_list_t display_list;
  const int32_t value;
} shdi_control_type_visual_enumeration_value_t;

typedef struct {
  const shdi_control_type_visual_enumeration_value_t *values;
  uint16_t size;
  uint32_t initial;
  uint8_t width;
} sdhi_control_type_visual_enumeration_t;

typedef union {
  const sdhi_control_type_integer_t integer;
  const sdhi_control_type_real_t real;
  sdhi_control_type_enumeration_t enumeration;
  sdhi_control_type_visual_enumeration_t visual_enumeration;
} sdhi_control_type_configuration_t;

typedef struct {
  uint16_t id;
  const char * const title;
  const int16_t group;
  const sdhi_control_type_t type;
  sdhi_control_type_configuration_t configuration;
} sdhi_control_t;


// First pointer is values array, second pointer is sdhi_t,
// but can not be passed as such, due to recursiveness of type
typedef display_list_t (*display_list_generator_t)(const int32_t * const, const void * const);

typedef struct {
  const int16_t id;
  const char * const title;
} sdhi_group_t;

typedef struct {
  const char * title;
  const char * subtitle;
  int32_t controls[8];
  const uint8_t generated_display;
  const display_list_generator_t display_list_generator;
} sdhi_panel_t;

typedef struct {
  const sdhi_control_t * const controls;
  const uint32_t controls_size;
  const sdhi_group_t * const groups;
  const uint32_t groups_size;
  const char * const panel_selector_title;
  const sdhi_panel_t * const panels;
  const uint32_t panels_size;
} sdhi_t;

void sdhi_init(const sdhi_t sdhi);
void sdhi_init_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi);
bool sdhi_update_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi);
void sdhi_update_displays(const int32_t * const values, const sdhi_t sdhi);
sdhi_control_type_t sdhi_type(const uint16_t id, const sdhi_t sdhi);
int32_t sdhi_integer(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
float sdhi_real(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
int32_t sdhi_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
int32_t sdhi_visual_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi);
