#pragma once
#include "midi.h"
#include "sdhi.h"
#include "action.h"
#include "i2c_controller.h"

typedef struct {
  sdhi_t sdhi;
  int32_t *values;
  i2c_controller_button_t *buttons;
  actions_t actions;
  action_value_t *action_values;
  midi_slot_t *midi_slots;
  uint8_t midi_slots_size;
} setup_t;
