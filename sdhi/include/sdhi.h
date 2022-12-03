#pragma once
#include "pico/stdlib.h"

typedef struct {
  const uint16_t id;
  const char * const title;
  const uint16_t group;
  const int32_t initial;
  const int32_t min;
  const int32_t max;
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
  const sdhi_panel_t * const panels;
  const uint32_t panels_size;
} sdhi_t;

void sdhi_init(const sdhi_t sdhi);
bool sdhi_update_values_blocking(int32_t * const values, const sdhi_t sdhi);