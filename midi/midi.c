#include <string.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/platform.h"
#include "midi.h"

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80

#define OUT_MESSAGES_SIZE 16
#define MAX_CONTROLLER_MESSAGES 8

typedef struct {
  uint8_t note;
  uint8_t channel;
} midi_mapped_note_t;

static uint8_t in_buffer[3];
static uint8_t in_position;
static queue_t in;

// Exclusive start + manufacturer id + max sysex data + exclusive end
static uint8_t out_buffer[1 + 3 + MIDI_EXCLUSIVE_MAX_LENGTH + 1];
static uint16_t out_position;
static uint8_t out_size;
static queue_t out;

static void read_note(midi_message_type_t type, midi_message_t *midi) {
  note_message_t midi_note = {
    in_buffer[0] & 0x0F,
    in_buffer[1] & 0x7F,
    in_buffer[2] & 0x7F
  };
  midi->type = type;
  midi->value.note = midi_note;
}

static void write_size(uint8_t size) {
  out_size = size;
  out_position = 0;
}

static void write_note(const note_message_t note_on, uint8_t status_prefix) {
  out_buffer[0] = status_prefix + (note_on.channel & 0x0F);
  out_buffer[1] = note_on.note & 0x7F;
  out_buffer[2] = note_on.velocity &0x7F;
  write_size(3);
}

static void write_controller(const controller_message_t controller) {
  out_buffer[0] = 0xB0 + (controller.channel & 0x0F);
  out_buffer[1] = controller.number & 0x7F;
  out_buffer[2] = controller.value & 0x7F;
  write_size(3);
}

static void write_program_change(const program_message_t program) {
  out_buffer[0] = 0xC0 + (program.channel & 0x0F);
  out_buffer[1] = program.number & 0x7F;
  write_size(2);
}

static void write_rpn(const rpn_message_t rpn, uint8_t msb_cc, uint8_t lsb_cc) {
    out_buffer[0] = 0xB0 + (rpn.channel & 0x0F);
    out_buffer[1] = msb_cc;
    out_buffer[2] = rpn.msb & 0x7F;
    out_buffer[3] = lsb_cc;
    out_buffer[4] = rpn.lsb & 0x7F;
    out_buffer[5] = 6;
    out_buffer[6] = rpn.value & 0x7F;
    out_buffer[7] = msb_cc;
    out_buffer[8] = 127;
    out_buffer[9] = lsb_cc;
    out_buffer[10] = 127;
    return write_size(11);
}

static void write_exclusive(const exclusive_message_t exclusive) {
  out_buffer[0] = 0xF0;
  uint8_t i;
  if(exclusive.manufacturer_id & 0xFF00 != 0) {
    out_buffer[1] = 0;
    out_buffer[2] = (exclusive.manufacturer_id >> 8) & 0xFF;
    out_buffer[3] = exclusive.manufacturer_id & 0xFF;
    i = 4;
  } else {
    out_buffer[1] = exclusive.manufacturer_id & 0xFF;
    i = 2;
  }
  memcpy(out_buffer + i, exclusive.data, exclusive.data_size);
  out_buffer[i + exclusive.data_size] = 0xF7;
  write_size(i + exclusive.data_size + 1);
}

static void write_raw(const raw_message_t raw) {
  out_buffer[0] = raw.x;
  out_buffer[1] = raw.y;
  out_buffer[2] = raw.z;
  write_size(3);
}

static void write_message(const midi_message_t message) {
  switch(message.type) {
  case MIDI_CONTROLLER_MESSAGE:
    write_controller(message.value.controller);
    return;
  case MIDI_NOTE_ON_MESSAGE:
    write_note(message.value.note, MIDI_NOTE_ON);
    return;
  case MIDI_NOTE_OFF_MESSAGE:
    write_note(message.value.note, MIDI_NOTE_OFF);
    return;
  case MIDI_PROGRAM_CHANGE_MESSAGE:
    write_program_change(message.value.program);
    return;
  case MIDI_RPN_MESSAGE:
    write_rpn(message.value.rpn, 101, 100);
    return;
  case MIDI_NRPN_MESSAGE:
    write_rpn(message.value.rpn, 99, 98);
    return;
  case MIDI_EXCLUSIVE_MESSAGE:
    write_exclusive(message.value.exclusive);
    return;
  case MIDI_RAW_MESSAGE:
    write_raw(message.value.raw);
    return;
  default:
    return;
  }
}

#define MIDI_NOTES 127

static midi_mapped_note_t not_mapped = {
  .note = 0x80
};
static midi_mapped_note_t mapping[MIDI_NOTES];

void midi_init() {
  in_position = 0;
  out_position = 0;
  out_size = 0;
  queue_init(&in, sizeof(midi_message_t), 32);
  queue_init(&out, sizeof(midi_message_t), OUT_MESSAGES_SIZE);

  for(uint8_t i = 0; i < MIDI_NOTES; i++) {
    mapping[i] = not_mapped;
  }

  uart_init(uart1, 31250);
  uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
  gpio_set_function(8, GPIO_FUNC_UART);
  gpio_set_function(9, GPIO_FUNC_UART);
}

void send_mapped(const midi_mapped_note_t map, const midi_message_type_t type, const uint8_t velocity) {
  midi_message_t message = {
    .type = type,
    .value.note = {
      .channel = map.channel,
      .note = map.note,
      .velocity = velocity
    }
  };
  if(!queue_try_add(&out, &message)) {
    panic("MIDI out queue is full!");
  }
}

void midi_run() {
  if(uart_is_readable(uart1)) {
    uart_read_blocking(uart1, in_buffer + in_position, 1);
    if(in_position == 2) {
      midi_message_t message;
      switch(in_buffer[0]) {
      case MIDI_NOTE_OFF: {
        read_note(MIDI_NOTE_OFF_MESSAGE, &message);
        midi_mapped_note_t map = mapping[message.value.note.note];
        if(map.note != 0x80) {
          send_mapped(map, MIDI_NOTE_OFF_MESSAGE, message.value.note.velocity);
        }
        in_position = 0;
        break;
      }
      case MIDI_NOTE_ON: {
        read_note(MIDI_NOTE_ON_MESSAGE, &message);
        midi_mapped_note_t map = mapping[message.value.note.note];
        if(map.note != 0x80) {
          send_mapped(map, MIDI_NOTE_ON_MESSAGE, message.value.note.velocity);
        }
        in_position = 0;
        break;
      }
      default:
        in_buffer[0] = in_buffer[1];
        in_buffer[1] = in_buffer[2];
        in_position = 2;
        break;
      }
    } else {
      in_position++;
    }
  } else if(uart_is_writable(uart1) && out_position < out_size) {
    uart_putc(uart1, out_buffer[out_position]);
    out_position++;
  } else if(out_position == out_size && queue_get_level(&out) != 0) {
    midi_message_t message;
    queue_remove_blocking(&out, &message);
    write_message(message);
  }
}

uint32_t midi_get_available_messages(midi_message_t * available, const uint32_t available_size) {
  for(uint32_t i = 0; i < available_size; i++) {
    if(!queue_try_remove(&in, available + i)) {
      return i;
    }
  }
}

uint32_t midi_can_send_messages() {
  return MIN(OUT_MESSAGES_SIZE - queue_get_level(&out), MAX_CONTROLLER_MESSAGES);
}

void midi_send_messages(midi_message_t * messages, const uint32_t messages_size) {
  for(uint32_t i = 0; i < messages_size; i++) {
    queue_add_blocking(&out, messages + i);
  }
}

void midi_set_mapped_note(const uint8_t note, const uint8_t out_channel, const uint8_t out_note) {
  const midi_mapped_note_t map = {
    .channel = out_channel,
    .note = out_note
  };
  mapping[note & 0x7F] = map;
}

void midi_clear_mapped_note(const uint8_t note) {
  mapping[note & 0x7F] = not_mapped;
}
