#pragma once
#include "midi.h"
#include "sdhi.hpp"
#include "action.hpp"
#include "i2c_controller.h"
namespace setup {

  struct Setup {
    sdhi::sdhi_t sdhi;
    int32_t *values;
    i2c_controller_button_t *buttons;
    tcb::span<action::Action> actions;
    action::StoredValue *action_values;
    midi_slot_t *midi_slots;
    uint8_t midi_slots_size;
    constexpr Setup(sdhi::sdhi_t sdhi, int32_t *values, i2c_controller_button_t *buttons, tcb::span<action::Action> actions, action::StoredValue *action_values, midi_slot_t *midi_slots, uint8_t midi_slots_size) : sdhi(sdhi), values(values), buttons(buttons), actions(actions), action_values(action_values), midi_slots(midi_slots), midi_slots_size(midi_slots_size) {}
  };
}
