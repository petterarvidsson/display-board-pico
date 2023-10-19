#pragma once
#include <stdint.h>
#include "sdhi.h"
#include "midi.h"

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
  PARAMETER_VALUE,
  PARAMETER_MIDI_NOTE
} parameter_type_t;

typedef enum {
  PARAMETER_MIDI_NOTE_VALUE,
  PARAMETER_MIDI_NOTE_VELOCITY,
  PARAMETER_MIDI_NOTE_STATE
} parameter_midi_note_parameter_t;

typedef struct {
  int16_t id;
  int32_t offset;
} parameter_control_t;

typedef struct {
  uint8_t slot;
  parameter_midi_note_parameter_t parameter;
} parameter_midi_note_t;

typedef struct {
  union {
    parameter_control_t control;
    int32_t value;
    parameter_midi_note_t note;
  } parameter;
  parameter_type_t type;
} parameter_t;

typedef enum {
  ACTION_CONTROLLER,
  ACTION_BANK_CHANGE,
  ACTION_NRPN,
  ACTION_MAPPING,
  ACTION_SLOT,
  ACTION_XG_PARAMETER_CHANGE_1,
  ACTION_YMF262_SLOT_STATE,
  ACTION_YMF262_PARAMETER,
  ACTION_YMF262_CONNECTION
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
  parameter_t slot;
} action_slot_configuration_t;

typedef struct {
  parameter_t parameter;
  parameter_t value;
} action_xg_parameter_change_1_configuration_t;

typedef struct {
  parameter_t slot;
  parameter_t state;
  parameter_t note;
} action_ymf262_slot_state_configuration_t;

typedef struct {
  parameter_t parameter;
  parameter_t value;
} action_ymf262_parameter_configuration_t;

typedef struct {
  parameter_t connection;
} action_ymf262_connection_configuration_t;

typedef union {
  action_controller_configuration_t controller;
  action_bank_change_configuration_t bank_change;
  action_rpn_configuration_t rpn;
  action_mapping_configuration_t mapping;
  action_slot_configuration_t slot;
  action_xg_parameter_change_1_configuration_t xg_parameter_change;
  action_ymf262_slot_state_configuration_t ymf262_slot_state;
  action_ymf262_parameter_configuration_t ymf262_parameter;
  action_ymf262_connection_configuration_t ymf262_connection;
} action_configuration_t;

typedef struct {
  uint8_t channel; // TODO: Not all actions require channel anymore
  const action_type_t type;
  action_configuration_t configuration;
} action_t;

typedef struct {
  const action_t * const actions;
  const uint8_t size;
} actions_t;

void action_init(const actions_t actions, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
void action_update(const actions_t actions, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size);
