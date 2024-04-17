#pragma once
#include "midi.h"
#include "sdhi.hpp"
#include "action.hpp"
#include "i2c_controller.h"

typedef struct {
  sdhi::sdhi_t sdhi;
  int32_t *values;
  i2c_controller_button_t *buttons;
  tcb::span<action::action_t> actions;
  action::action_value_t *action_values;
  midi_slot_t *midi_slots;
  uint8_t midi_slots_size;
} setup_t;
