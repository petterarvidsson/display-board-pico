#pragma once
#include "pico/stdlib.h"

typedef enum {
  SDHI_CONTROL_TYPE_INTEGER
} sdhi_control_type_t;

typedef struct {
  const int32_t min;
  const int32_t max;
} sdhi_control_type_integer_t;

typedef union {
  const sdhi_control_type_integer_t integer;
} sdhi_control_type_configuration_t;

typedef struct {
  const uint16_t id;
  const char * const title;
  const uint16_t group;
  const sdhi_control_type_t type;
  const sdhi_control_type_configuration_t configuration;
} sdhi_control_t;

typedef struct {
  const uint16_t id;
  const char * const title;
} sdhi_group_t;

typedef struct {
  const char * const title;
  const int32_t const controls[8];
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
bool sdhi_update_values_blocking(int32_t * const values, const sdhi_t sdhi);
void sdhi_update_displays(const int32_t * const values, const sdhi_t sdhi);
