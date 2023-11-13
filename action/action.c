#include <string.h>
#include "action.h"
#include "midi.h"
#include "sdhi.h"
#include "ymf262.h"

typedef struct {
  const midi_slot_t * const slots;
  const uint8_t size;
} slots_t;

static bool v_eq(const value_t v1, const value_t v2) {
  return v1.v1 == v2.v1 && v1.v2 == v2.v2 && v1.v3 == v2.v3 && v1.trigger == v2.trigger;
}

static bool value_eq(const action_value_t value) {
  return v_eq(value.computed, value.sent);
}

static uint8_t current_action;

static midi_message_t action_messages[8];

static uint8_t execute_action_controller(const uint8_t channel, const value_t value, midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = value.v1 & 0x7F,
      .value = value.v2 & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}


static uint8_t execute_action_rpn(const uint8_t channel, const value_t value, midi_message_t * const to_send) {
  midi_message_t message = {
    .type = MIDI_NRPN_MESSAGE,
    .value.rpn = {
      .channel = channel & 0x7F,
      .msb = value.v1 & 0x7F,
      .lsb = value.v2 & 0x7F,
      .value = value.v3 & 0x7F
    }
  };
  to_send[0] = message;
  return 1;
}

static uint8_t execute_action_xg_parameter_change_1(const uint8_t channel, const value_t value, midi_message_t * const to_send) {
  uint8_t data[MIDI_EXCLUSIVE_MAX_LENGTH];

  data[0] = 0x08;
  data[1] = channel & 0x7F;
  data[2] = value.v1 & 0x7F;
  data[3] = value.v2 & 0x7F;

  midi_message_t message = {
    .type = MIDI_EXCLUSIVE_MESSAGE,
    .value.exclusive = {
      .channel = channel & 0x7F,
      .manufacturer_id = 0x43,
      .data_size = 3
    }
  };
  memcpy(message.value.exclusive.data, data, message.value.exclusive.data_size);

  to_send[0] = message;
  return 1;
}

static uint8_t execute_action_bank_change(const uint8_t channel, const value_t value, midi_message_t * const to_send) {
  const midi_message_t c1 = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = 0,
      .value = 127
    }
  };
  const midi_message_t c2 = {
      .type = MIDI_CONTROLLER_MESSAGE,
      .value.controller = {
        .channel = channel & 0x7F,
        .number = 32,
        .value = 0
      }
  };
  const midi_message_t p = {
    .type = MIDI_PROGRAM_CHANGE_MESSAGE,
    .value.controller = {
      .channel = channel & 0x7F,
      .number = value.v1 & 0x7F
    }
  };
  to_send[0] = c1;
  to_send[1] = c2;
  to_send[2] = p;
  return 3;
}

static void execute_action_mapping(const uint8_t channel, const value_t value) {
  midi_set_mapped_note(value.v1 & 0x7F, channel & 0x7F, value.v2 & 0x7F);
}

static void execute_action_slot(const uint8_t channel, const value_t value) {
  midi_set_slot_for_channel(channel & 0x7F, value.v1 & 0x7F);
}

static void execute_action_ymf262_slot_state(const value_t value, uint8_t * trigger_ymf262_channel) {
  const uint8_t slot = value.v1 & 0x07;
  const uint8_t note = value.v3 & 0x7F;
  const float frequency = midi_note_to_frequency(note);
  ymf262_stop(slot);
  ymf262_frequency(slot, frequency);
  if(value.v2 != 0) {
    *trigger_ymf262_channel |= 1 << slot;
  }
}

static void execute_action_ymf262_parameter(const value_t value) {
  const ymf262_parameter_t parameter = value.v1 & 0x7FFFFFF;
  const uint8_t parameter_value = value.v2 & 0xFF;
  ymf262_all_channels_parameter(parameter, parameter_value);
}

static void execute_action_ymf262_connection(const value_t value) {
  const ymf262_channel_connection_t connection = value.v1 & 0xF;
  ymf262_all_channels_connection(connection);
}

static int32_t buf[64];

static void execute_action_load_values(const action_load_values_t action, const value_t value, int32_t * const values) {
  if (value.trigger) {
    const uint8_t patch = value.v1 & 0x7F;
    load_patch(patch, buf, action.ids_size);

    for(uint8_t i = 0; i < action.ids_size; i++) {
      const uint16_t id = action.ids[i];
      values[id] = buf[i];
    }
  }
}

static void execute_action_save_values(const action_save_values_t action, const value_t value, int32_t * const values) {
  if (value.trigger) {
    const uint8_t patch = value.v1 & 0x7F;
    for(uint8_t i = 0; i < action.ids_size; i++) {
      const uint16_t id = action.ids[i];
      buf[i] = values[id];
    }
    save_patch(patch, buf, action.ids_size);
  }
}

static bool execute_action(const action_t action, const value_t value, uint8_t * trigger_ymf262_channel, int32_t * const values) {
  uint8_t messages = 0;
  switch(action.type) {
  case ACTION_CONTROLLER:
    messages = execute_action_controller(action.channel, value, action_messages);
    break;
  case ACTION_NRPN:
    messages = execute_action_rpn(action.channel, value, action_messages);
    break;
  case ACTION_BANK_CHANGE:
    messages = execute_action_bank_change(action.channel, value, action_messages);
    break;
  case ACTION_MAPPING:
    execute_action_mapping(action.channel, value);
    messages = 0;
    break;
  case ACTION_SLOT:
    execute_action_slot(action.channel, value);
    messages = 0;
    break;
  case ACTION_XG_PARAMETER_CHANGE_1:
    messages = execute_action_xg_parameter_change_1(action.channel, value, action_messages);
    break;
  case ACTION_YMF262_SLOT_STATE:
    messages = 0;
    execute_action_ymf262_slot_state(value, trigger_ymf262_channel);
    break;
  case ACTION_YMF262_PARAMETER:
    messages = 0;
    execute_action_ymf262_parameter(value);
    break;
  case ACTION_YMF262_CONNECTION:
    messages = 0;
    execute_action_ymf262_connection(value);
    break;
  case ACTION_LOAD_VALUES:
    messages = 0;
    execute_action_load_values(action.configuration.load_values, value, values);
    break;
  case ACTION_SAVE_VALUES:
    messages = 0;
    execute_action_save_values(action.configuration.save_values, value, values);
    break;
  }
  if(messages < midi_can_send_messages()) {
    midi_send_messages(action_messages, messages);
    return true;
  } else {
    return false;
  }
}

static void execute_actions(const action_t * const actions, const uint8_t actions_size, action_value_t * action_values, int32_t * const values) {
  if(actions_size > 0) {
    uint8_t last_action;
    uint8_t trigger_ymf262_channel = 0;

    if(current_action == 0) {
      last_action = actions_size - 1;
    } else {
      last_action = current_action - 1;
    }

    for(; current_action != last_action; current_action = (current_action + 1) % actions_size) {
      if(!value_eq(action_values[current_action]) && !execute_action(actions[current_action], action_values[current_action].computed, &trigger_ymf262_channel, values)) {
        break;
      } else {
        action_values[current_action].sent = action_values[current_action].computed;
      }
    }

    for(uint8_t c = 0; c < 6; c++) {
      if((trigger_ymf262_channel >> c) & 0x1) {
        ymf262_start(c);
      }
    }
  }
}

static int32_t parameter_value(const parameter_t parameter, const sdhi_t sdhi, const int32_t * const values, const slots_t slots) {
  int32_t value = -1;
  switch(parameter.type) {
  case PARAMETER_CONTROL:
    switch(sdhi_type(parameter.parameter.control.id, sdhi)) {
    case SDHI_CONTROL_TYPE_INTEGER:
      value = sdhi_integer(parameter.parameter.control.id, values, sdhi) + parameter.parameter.control.offset;
      break;
    case SDHI_CONTROL_TYPE_ENUMERATION:
      value = sdhi_enumeration(parameter.parameter.control.id, values, sdhi) + parameter.parameter.control.offset;
      break;
    case SDHI_CONTROL_TYPE_REAL:
      break;
    }
    break;
  case PARAMETER_VALUE:
    value = parameter.parameter.value;
    break;
  case PARAMETER_MIDI_NOTE:
    if(parameter.parameter.note.slot < slots.size) {
      switch(parameter.parameter.note.parameter) {
      case PARAMETER_MIDI_NOTE_VALUE:
        value = slots.slots[parameter.parameter.note.slot].note;
        break;
      case PARAMETER_MIDI_NOTE_VELOCITY:
        value = slots.slots[parameter.parameter.note.slot].velocity;
        break;
      case PARAMETER_MIDI_NOTE_STATE:
        value = slots.slots[parameter.parameter.note.slot].on;
        break;
      }
    }
    break;
  }
  return value;
}

static void trigger_value(const trigger_button_t trigger, const i2c_controller_button_t * const button, uint8_t * const value) {
  const i2c_controller_button_t  b = button[trigger.id];
  if(b == I2C_CONTROLLER_PRESSED) {
     *value = 1;
  } else if(b == I2C_CONTROLLER_RELEASED) {
     *value = 0;
  }
}

static void update_computed_values(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const slots_t slots) {
  for(uint8_t i = 0; i < actions_size; i++) {
    const action_t action = actions[i];
    switch(action.type) {
    case ACTION_CONTROLLER:
      action_values[i].computed.v1 = parameter_value(action.configuration.controller.number, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.controller.value, sdhi, values, slots);
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_NRPN:
      action_values[i].computed.v1 = parameter_value(action.configuration.rpn.msb, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.rpn.lsb, sdhi, values, slots);
      action_values[i].computed.v3 = parameter_value(action.configuration.rpn.value, sdhi, values, slots);
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_BANK_CHANGE:
      action_values[i].computed.v1 = parameter_value(action.configuration.bank_change.value, sdhi, values, slots);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_MAPPING:
      action_values[i].computed.v1 = parameter_value(action.configuration.mapping.note, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.mapping.value, sdhi, values, slots);
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_SLOT:
      action_values[i].computed.v1 = parameter_value(action.configuration.slot.slot, sdhi, values, slots);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      break;
    case ACTION_XG_PARAMETER_CHANGE_1:
      action_values[i].computed.v1 = parameter_value(action.configuration.xg_parameter_change.parameter, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.xg_parameter_change.value, sdhi, values, slots);
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_YMF262_SLOT_STATE:
      action_values[i].computed.v1 = parameter_value(action.configuration.ymf262_slot_state.slot, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.ymf262_slot_state.state, sdhi, values, slots);
      action_values[i].computed.v3 = parameter_value(action.configuration.ymf262_slot_state.note, sdhi, values, slots);
      break;
    case ACTION_YMF262_PARAMETER:
      action_values[i].computed.v1 = parameter_value(action.configuration.ymf262_parameter.parameter, sdhi, values, slots);
      action_values[i].computed.v2 = parameter_value(action.configuration.ymf262_parameter.value, sdhi, values, slots);
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_YMF262_CONNECTION:
      action_values[i].computed.v1 = parameter_value(action.configuration.ymf262_connection.connection, sdhi, values, slots);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      action_values[i].computed.trigger = 0;
      break;
    case ACTION_LOAD_VALUES:
      action_values[i].computed.v1 = parameter_value(action.configuration.load_values.patch, sdhi, values, slots);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      trigger_value(action.configuration.load_values.trigger, button, &(action_values[i].computed.trigger));
      break;
    case ACTION_SAVE_VALUES:
      action_values[i].computed.v1 = parameter_value(action.configuration.save_values.patch, sdhi, values, slots);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      trigger_value(action.configuration.save_values.trigger, button, &(action_values[i].computed.trigger));
      break;
    }
  }
}

void action_init(const actions_t const actions, const sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size) {
  current_action = 0;
  slots_t internal = {
    .slots = slots,
    .size = slots_size
  };
  update_computed_values(actions.actions, actions.size, sdhi, values, button, action_values, internal);

  // Not used for init, we will not trigger any notes on
  uint8_t trigger_ymf262_channel = 0;
  for(uint8_t i; i < actions.size; i++) {
    while(!execute_action(actions.actions[i], action_values[i].computed, &trigger_ymf262_channel, values)) {
      sleep_ms(10);
    }
    action_values[i].sent = action_values[i].computed;
  }
}

void action_update(const actions_t actions, const sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, action_value_t * action_values, const midi_slot_t * const slots, const uint8_t slots_size) {
  slots_t internal = {
    .slots = slots,
    .size = slots_size
  };
  update_computed_values(actions.actions, actions.size, sdhi, values, button, action_values, internal);
  execute_actions(actions.actions, actions.size, action_values, values);
}
