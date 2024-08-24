#include <cstring>
#include "action.hpp"
#include "midi.h"
#include "ymf262.h"

namespace action {
  typedef struct {
    const midi_slot_t * const slots;
    const uint8_t size;
  } slots_t;

  static uint8_t current_action;

  static midi_message_t action_messages[8];
  static int32_t buf[64];

  bool operator== (const Value lhs, const Value rhs) {
    return lhs.v1 == rhs.v1 && lhs.v2 == rhs.v2 && lhs.v3 == rhs.v3 && lhs.trigger == rhs.trigger;
  }

  static bool value_eq(const StoredValue value) {
    return value.computed == value.sent;
  }

  class ActionExecutor {
    const Value value;
    midi_message_t * const to_send;
    uint8_t * trigger_ymf262_channel;
    int32_t * const values;
  public:
    uint8_t messages;
    ActionExecutor(const Value value, midi_message_t * const to_send, uint8_t * trigger_ymf262_channel, int32_t * const values) : value(value), to_send(to_send), trigger_ymf262_channel(trigger_ymf262_channel), values(values), messages(0) {}

    void operator()(const MidiCC c) {
      midi_message_t message;

      message.type = MIDI_CONTROLLER_MESSAGE;
      message.value.controller = {
        .channel = (uint8_t)(c.channel & 0x7F),
        .number = (uint8_t)(value.v1 & 0x7F),
        .value = (uint8_t)(value.v2 & 0x7F)
      };
      to_send[0] = message;
      messages = 1;
    }
    void operator()(MidiBank b) {
      midi_message_t c1;
      c1.type = MIDI_CONTROLLER_MESSAGE;
      c1.value.controller = {
        .channel = (uint8_t)(b.channel & 0x7F),
        .number = 0,
        .value = 127
      };
      midi_message_t c2;
      c2.type = MIDI_CONTROLLER_MESSAGE;
      c2.value.controller = {
        .channel = (uint8_t)(b.channel & 0x7F),
        .number = 32,
        .value = 0
      };
      midi_message_t p;
      p.type = MIDI_PROGRAM_CHANGE_MESSAGE;
      p.value.controller = {
        .channel = (uint8_t)(b.channel & 0x7F),
        .number = (uint8_t)(value.v1 & 0x7F)
      };
      to_send[0] = c1;
      to_send[1] = c2;
      to_send[2] = p;
      messages = 3;
    }
    void operator()(MidiRPN rpn) {
      midi_message_t message;
      message.type = MIDI_NRPN_MESSAGE;
      message.value.rpn = {
        .channel = (uint8_t)(rpn.channel & 0x7F),
        .msb = (uint8_t)(value.v1 & 0x7F),
        .lsb = (uint8_t)(value.v2 & 0x7F),
        .value = (uint8_t)(value.v3 & 0x7F)
      };
      to_send[0] = message;
      messages = 1;
    }
    void operator()(MidiNRPN nrpn) {
      midi_message_t message;
      message.type = MIDI_RPN_MESSAGE;
      message.value.rpn = {
        .channel = (uint8_t)(nrpn.channel & 0x7F),
        .msb = (uint8_t)(value.v1 & 0x7F),
        .lsb = (uint8_t)(value.v2 & 0x7F),
        .value = (uint8_t)(value.v3 & 0x7F)
      };
      to_send[0] = message;
      messages = 1;
    }
    void operator()(MidiMapping c) {
      midi_set_mapped_note((uint8_t)(value.v1 & 0x7F), c.channel & 0x7F, (uint8_t)(value.v2 & 0x7F));
    }
    void operator()(Slot s) {
      midi_set_slot_for_channel(s.channel & 0x7F, (uint8_t)(value.v1 & 0x7F));
    }
    void operator()(XGParameter p) {
      uint8_t data[MIDI_EXCLUSIVE_MAX_LENGTH];

      data[0] = 0x08;
      data[1] = p.channel & 0x7F;
      data[2] = (uint8_t)(value.v1 & 0x7F);
      data[3] = (uint8_t)(value.v2 & 0x7F);

      midi_message_t message;
      message.type = MIDI_EXCLUSIVE_MESSAGE;
      message.value.exclusive.channel = (uint8_t)(p.channel & 0x7F);
      message.value.exclusive.manufacturer_id = 0x43;
      message.value.exclusive.data_size = 3;
      memcpy(message.value.exclusive.data, data, message.value.exclusive.data_size);
      to_send[0] = message;
      messages = 1;
    }
    void operator()(YMF262Slot s) {
      const uint8_t slot = (uint8_t)(value.v1 & 0x07);
      const uint8_t note = (uint8_t)(value.v3 & 0x7F);
      const float frequency = midi_note_to_frequency(note);
      ymf262_stop(slot);
      ymf262_frequency(slot, frequency);
      if(value.v2 != 0) {
        *trigger_ymf262_channel |= 1 << slot;
      }
    }
    void operator()(YMF262Parameter c) {
      const ymf262_parameter_t parameter = (ymf262_parameter_t)(value.v1 & 0x7FFFFFF);
      const uint8_t parameter_value = (uint8_t)(value.v2 & 0xFF);
      ymf262_all_channels_parameter(parameter, parameter_value);
    }
    void operator()(YMF262Connection c) {
      const ymf262_channel_connection_t connection = (ymf262_channel_connection_t)(value.v1 & 0xF);
      ymf262_all_channels_connection(connection);
    }
    void operator()(Load load) {
      if (value.trigger) {
        const uint8_t patch = value.v1 & 0x7F;
        load_patch(patch, buf, load.ids_size);

        for(uint8_t i = 0; i < load.ids_size; i++) {
          const uint16_t id = load.ids[i];
          values[id] = buf[i];
        }
      }
    }
    void operator()(Save save) {
      if (value.trigger) {
        const uint8_t patch = value.v1 & 0x7F;
        for(uint8_t i = 0; i < save.ids_size; i++) {
          const uint16_t id = save.ids[i];
          buf[i] = values[id];
        }
        save_patch(patch, buf, save.ids_size);
      }
    }

  };
  static bool execute_action(const action_t action, const Value value, uint8_t * trigger_ymf262_channel, int32_t * const values) {
    uint8_t messages = 0;
    ActionExecutor executor(value, action_messages, trigger_ymf262_channel, values);

    mapbox::util::apply_visitor(executor, action);

    if(executor.messages < midi_can_send_messages()) {
      midi_send_messages(action_messages, executor.messages);
      return true;
    } else {
      return false;
    }
  }

  static void execute_actions(const tcb::span<const action_t> actions, StoredValue * action_values, int32_t * const values) {
    if(!actions.empty()) {
      uint8_t last_action;
      uint8_t trigger_ymf262_channel = 0;

      if(current_action == 0) {
        last_action = actions.size() - 1;
      } else {
        last_action = current_action - 1;
      }

      for(; current_action != last_action; current_action = (current_action + 1) % actions.size()) {
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
  static int32_t parameter_value(const Parameter parameter, const sdhi::sdhi_t sdhi, const int32_t * const values, const slots_t slots) {
    int32_t value = -1;
    parameter.match(
        [&value, sdhi, values] (ParameterControl control) {
          const sdhi::sdhi_control_t sdhi_control = *sdhi::find_control(control.id, sdhi);
          sdhi_control.match(
                             [&value, sdhi, values, control] (sdhi::sdhi_control_type_integer_t integer_control) {
                               value = sdhi_integer(control.id, values, sdhi) + control.offset;
                             },
                             [&value, sdhi, values, control] (sdhi::sdhi_control_type_real_t real_control) {
                             },
                             [&value, sdhi, values, control] (sdhi::sdhi_control_type_enumeration_t enumeration_control) {
                               value = sdhi_enumeration(control.id, values, sdhi) + control.offset;
                             },
                             [&value, sdhi, values, control] (sdhi::sdhi_control_type_visual_enumeration_t visual_enumeration_control) {
                               value = sdhi_visual_enumeration(control.id, values, sdhi) + control.offset;
                             });
        },
        [&value] (int32_t parameter) {
          value = parameter;
        },
        [&value, slots] (ParameterMidiNote note) {
          if(note.slot < slots.size) {
            switch(note.type) {
            case VALUE:
              value = slots.slots[note.slot].note;
              break;
            case VELOCITY:
              value = slots.slots[note.slot].velocity;
              break;
            case STATE:
              value = slots.slots[note.slot].on;
              break;
            }
          }
        }
      );
    return value;
  }

  static void trigger_value(const Trigger trigger, const i2c_controller_button_t * const button, uint8_t * const value) {
    const i2c_controller_button_t  b = button[trigger.id];
    if(b == I2C_CONTROLLER_PRESSED) {
      *value = 1;
    } else if(b == I2C_CONTROLLER_RELEASED) {
      *value = 0;
    }
  }

  static void update_computed_values(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, const int32_t * const values, const i2c_controller_button_t * const button, StoredValue * action_values, const slots_t slots) {
    for(uint8_t i = 0; i < actions.size(); i++) {
      actions[i].match(
                   [i, action_values, sdhi, values, slots] (MidiCC controller) {
                     action_values[i].computed.v1 = parameter_value(controller.number, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(controller.value, sdhi, values, slots);
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (MidiBank bank_change) {
                     action_values[i].computed.v1 = parameter_value(bank_change.value, sdhi, values, slots);
                     action_values[i].computed.v2 = 0;
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (MidiRPN rpn) {
                     action_values[i].computed.v1 = parameter_value(rpn.msb, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(rpn.lsb, sdhi, values, slots);
                     action_values[i].computed.v3 = parameter_value(rpn.value, sdhi, values, slots);
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (MidiNRPN nrpn) {
                     action_values[i].computed.v1 = parameter_value(nrpn.msb, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(nrpn.lsb, sdhi, values, slots);
                     action_values[i].computed.v3 = parameter_value(nrpn.value, sdhi, values, slots);
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (MidiMapping mapping) {
                     action_values[i].computed.v1 = parameter_value(mapping.note, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(mapping.value, sdhi, values, slots);
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (Slot slot) {
                     action_values[i].computed.v1 = parameter_value(slot.slot, sdhi, values, slots);
                     action_values[i].computed.v2 = 0;
                     action_values[i].computed.v3 = 0;
                   },
                   [i, action_values, sdhi, values, slots] (XGParameter xg_parameter_change) {
                     action_values[i].computed.v1 = parameter_value(xg_parameter_change.parameter, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(xg_parameter_change.value, sdhi, values, slots);
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (YMF262Slot ymf262_slot_state) {
                     action_values[i].computed.v1 = parameter_value(ymf262_slot_state.slot, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(ymf262_slot_state.state, sdhi, values, slots);
                     action_values[i].computed.v3 = parameter_value(ymf262_slot_state.note, sdhi, values, slots);
                   },
                   [i, action_values, sdhi, values, slots] (YMF262Parameter ymf262_parameter) {
                     action_values[i].computed.v1 = parameter_value(ymf262_parameter.parameter, sdhi, values, slots);
                     action_values[i].computed.v2 = parameter_value(ymf262_parameter.value, sdhi, values, slots);
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots] (YMF262Connection connection) {
                     action_values[i].computed.v1 = parameter_value(connection.connection, sdhi, values, slots);
                     action_values[i].computed.v2 = 0;
                     action_values[i].computed.v3 = 0;
                     action_values[i].computed.trigger = 0;
                   },
                   [i, action_values, sdhi, values, slots, button] (Load load) {
                     action_values[i].computed.v1 = parameter_value(load.patch, sdhi, values, slots);
                     action_values[i].computed.v2 = 0;
                     action_values[i].computed.v3 = 0;
                     trigger_value(load.trigger, button, &(action_values[i].computed.trigger));
                   },
                   [i, action_values, sdhi, values, slots, button] (Save save) {
                     action_values[i].computed.v1 = parameter_value(save.patch, sdhi, values, slots);
                     action_values[i].computed.v2 = 0;
                     action_values[i].computed.v3 = 0;
                     trigger_value(save.trigger, button, &(action_values[i].computed.trigger));
                   });
    }
  }

  void action_init(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, StoredValue * action_values, const midi_slot_t * const slots, const uint8_t slots_size) {
    current_action = 0;
    slots_t internal = {
      .slots = slots,
      .size = slots_size
    };
    update_computed_values(actions, sdhi, values, button, action_values, internal);

    // Not used for init, we will not trigger any notes on
    uint8_t trigger_ymf262_channel = 0;
    for(uint8_t i; i < actions.size(); i++) {
      while(!execute_action(actions[i], action_values[i].computed, &trigger_ymf262_channel, values)) {
        sleep_ms(10);
      }
      action_values[i].sent = action_values[i].computed;
    }
  }

  void action_update(const tcb::span<const action_t> actions, const sdhi::sdhi_t sdhi, int32_t * const values, const i2c_controller_button_t * const button, StoredValue * action_values, const midi_slot_t * const slots, const uint8_t slots_size) {
    slots_t internal = {
      .slots = slots,
      .size = slots_size
    };
    update_computed_values(actions, sdhi, values, button, action_values, internal);
    execute_actions(actions, action_values, values);
  }
};
