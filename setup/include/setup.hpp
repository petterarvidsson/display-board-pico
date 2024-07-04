#pragma once
#include "midi.h"
#include "sdhi.hpp"
#include "action.hpp"
#include "i2c_controller.h"
namespace setup {

  namespace dl = display_list;
  using namespace sdhi;
  using namespace tcb;
  using namespace action;
  struct Setup {
    sdhi_t sdhi;
    int32_t *values;
    i2c_controller_button_t *buttons;
    span<action_t> actions;
    action_value_t *action_values;
    midi_slot_t *midi_slots;
    uint8_t midi_slots_size;
  };
}
