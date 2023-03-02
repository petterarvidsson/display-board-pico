#pragma once
#include <stdint.h>
#include "sdhi.h"

typedef struct {
  int32_t v1;
  int32_t v2;
  int32_t v3;
} value_t;

typedef struct  {
  value_t computed;
  value_t sent;
} action_value_t;

typedef enum {
  PARAMETER_CONTROL,
  PARAMETER_VALUE
} parameter_type_t;

typedef struct {
  int16_t id;
  int32_t offset;
} parameter_control_t;

typedef struct {
  union {
    parameter_control_t control;
    int32_t value;
  } parameter;
  parameter_type_t type;
} parameter_t;

typedef enum {
  ACTION_CONTROLLER,
  ACTION_BANK_CHANGE,
  ACTION_NRPN,
  ACTION_MAPPING,
  ACTION_XG_PARAMETER_CHANGE_1
} action_type_t;

typedef struct {
  parameter_t number;
  parameter_t value;
} action_controller_configuration_t;

typedef struct {
  parameter_t value;
} action_bank_change_configuration_t;

typedef struct {
  parameter_t msb;
  parameter_t lsb;
  parameter_t value;
} action_rpn_configuration_t;

typedef struct {
  parameter_t note;
  parameter_t value;
} action_mapping_configuration_t;

typedef struct {
  parameter_t parameter;
  parameter_t value;
} action_xg_parameter_change_1_configuration_t;

typedef union {
  action_controller_configuration_t controller;
  action_bank_change_configuration_t bank_change;
  action_rpn_configuration_t rpn;
  action_mapping_configuration_t mapping;
  action_xg_parameter_change_1_configuration_t xg_parameter_change;
} action_configuration_t;

typedef struct {
  uint8_t channel;
  const action_type_t type;
  action_configuration_t configuration;
} action_t;

void action_init(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values);
void action_update(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values);
