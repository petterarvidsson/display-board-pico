#include <string.h>
#include "action.h"
#include "midi.h"
#include "sdhi.h"

static bool v_eq(const value_t v1, const value_t v2) {
  return v1.v1 == v2.v1 && v1.v2 == v2.v2 && v1.v3 == v2.v3;
}

static bool value_eq(const action_value_t value) {
  return v_eq(value.computed, value.sent);
}

static uint8_t current_action;

static midi_message_t action_messages[8];

static uint8_t execute_action_controller(const action_controller_configuration_t configuration, const uint8_t channel, const value_t value, midi_message_t * const to_send) {
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


static uint8_t execute_action_rpn(const action_rpn_configuration_t configuration, const uint8_t channel, const value_t value, midi_message_t * const to_send) {
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

static uint8_t execute_action_xg_parameter_change_1(const action_xg_parameter_change_1_configuration_t configuration, const uint8_t channel, const value_t value, midi_message_t * const to_send) {
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

static void execute_action_mapping(const action_mapping_configuration_t configuration, const uint8_t channel, const value_t value) {
  midi_set_mapped_note(value.v1 & 0x7F, channel & 0x7F, value.v2 & 0x7F);
}

static bool execute_action(const action_t action, const value_t value) {
  uint8_t messages = 0;
  switch(action.type) {
  case ACTION_CONTROLLER:
    messages = execute_action_controller(action.configuration.controller, action.channel, value, action_messages);
    break;
  case ACTION_NRPN:
    messages = execute_action_rpn(action.configuration.rpn, action.channel, value, action_messages);
    break;
  case ACTION_BANK_CHANGE:
    messages = execute_action_bank_change(action.channel, value, action_messages);
    break;
  case ACTION_MAPPING:
    execute_action_mapping(action.configuration.mapping, action.channel, value);
    messages = 0;
    break;
  case ACTION_XG_PARAMETER_CHANGE_1:
    messages = execute_action_xg_parameter_change_1(action.configuration.xg_parameter_change, action.channel, value, action_messages);
    break;
  }
  if(messages < midi_can_send_messages()) {
    midi_send_messages(action_messages, messages);
    return true;
  } else {
    return false;
  }
}

static void execute_actions(const action_t * const actions, const uint8_t actions_size, action_value_t * action_values) {
  uint8_t last_action;
  if(current_action == 0) {
    last_action = actions_size - 1;
  } else {
    last_action = current_action - 1;
  }

  for(; current_action != last_action; current_action = (current_action + 1) % actions_size) {
    if(!value_eq(action_values[current_action]) && !execute_action(actions[current_action], action_values[current_action].computed)) {
      break;
    } else {
      action_values[current_action].sent = action_values[current_action].computed;
    }
  }
}

static int32_t parameter_value(const parameter_t parameter, const sdhi_t sdhi, const int32_t * const values) {
  int32_t value = -1;
  switch(parameter.type) {
  case PARAMETER_CONTROL:
    switch(sdhi_type(parameter.parameter.control.id, sdhi)) {
    case SDHI_CONTROL_TYPE_INTEGER:
      value = sdhi_integer(parameter.parameter.control.id, values, sdhi) + parameter.parameter.control.offset;
      break;
    case SDHI_CONTROL_TYPE_ENUMERATION:
      value = sdhi_enumeration(parameter.parameter.control.id, values, sdhi)  + parameter.parameter.control.offset;
      break;
    case SDHI_CONTROL_TYPE_REAL:
      break;
    }
    break;
  case PARAMETER_VALUE:
    value = parameter.parameter.value;
    break;
  }
  return value;
}

static void update_computed_values(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values) {
  for(uint8_t i = 0; i < actions_size; i++) {
    const action_t action = actions[i];
    switch(action.type) {
    case ACTION_CONTROLLER:
      action_values[i].computed.v1 = parameter_value(action.configuration.controller.number, sdhi, values);
      action_values[i].computed.v2 = parameter_value(action.configuration.controller.value, sdhi, values);
      action_values[i].computed.v3 = 0;
      break;
    case ACTION_NRPN:
      action_values[i].computed.v1 = parameter_value(action.configuration.rpn.msb, sdhi, values);
      action_values[i].computed.v2 = parameter_value(action.configuration.rpn.lsb, sdhi, values);
      action_values[i].computed.v3 = parameter_value(action.configuration.rpn.value, sdhi, values);
      break;
    case ACTION_BANK_CHANGE:
      action_values[i].computed.v1 = parameter_value(action.configuration.bank_change.value, sdhi, values);
      action_values[i].computed.v2 = 0;
      action_values[i].computed.v3 = 0;
      break;
    case ACTION_MAPPING:
      action_values[i].computed.v1 = parameter_value(action.configuration.mapping.note, sdhi, values);
      action_values[i].computed.v2 = parameter_value(action.configuration.mapping.value, sdhi, values);
      action_values[i].computed.v3 = 0;
      break;
    case ACTION_XG_PARAMETER_CHANGE_1:
      action_values[i].computed.v1 = parameter_value(action.configuration.xg_parameter_change.parameter, sdhi, values);
      action_values[i].computed.v2 = parameter_value(action.configuration.xg_parameter_change.value, sdhi, values);
      action_values[i].computed.v3 = 0;
      break;
    }
  }
}

void action_init(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values) {
  current_action = 0;
  update_computed_values(actions, actions_size, sdhi, values, action_values);
  for(uint8_t i; i < actions_size; i++) {
    while(!execute_action(actions[i], action_values[i].computed)) {
      sleep_ms(10);
    }
    action_values[i].sent = action_values[i].computed;
  }
}

void action_update(const action_t * const actions, const uint8_t actions_size, const sdhi_t sdhi, const int32_t * const values, action_value_t * action_values) {
  update_computed_values(actions, actions_size, sdhi, values, action_values);
  execute_actions(actions, actions_size, action_values);
}
