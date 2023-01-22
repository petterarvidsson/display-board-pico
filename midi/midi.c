#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/platform.h"
#include "midi.h"

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80

#define OUT_MESSAGES_SIZE 8

static uint8_t in_buffer[3];
static uint8_t in_position;
static queue_t in;

static uint8_t out_buffer[3];
static uint8_t out_position;
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

static void read_raw(midi_message_t *midi) {
  raw_message_t raw = {
    .x = in_buffer[0],
    .y = in_buffer[1],
    .z = in_buffer[2]
  };
  midi->type = MIDI_RAW_MESSAGE;
  midi->value.raw = raw;
}

static void write_size(uint8_t size) {
  out_size = size;
  out_position = 0;
}

static uint32_t midi_write_note(const midi_message_t message, uint8_t status_prefix) {
  note_message_t note_on = message.value.note;
  out_buffer[0] = status_prefix + (note_on.channel & 0x0F);
  out_buffer[1] = note_on.note & 0x7F;
  out_buffer[2] = note_on.velocity &0x7F;
  write_size(3);
}

static uint32_t midi_write_raw(const midi_message_t message) {
  raw_message_t raw = message.value.raw;
  out_buffer[0] = raw.x;
  out_buffer[1] = raw.y;
  out_buffer[2] = raw.z;
  write_size(3);
}

void midi_write_message(const midi_message_t message) {
  switch(message.type) {
  case MIDI_NOTE_ON_MESSAGE:
    midi_write_note(message, MIDI_NOTE_ON);
    return;
  case MIDI_NOTE_OFF_MESSAGE:
    midi_write_note(message, MIDI_NOTE_OFF);
    return;
  case MIDI_RAW_MESSAGE:
    midi_write_raw(message);
    return;
  default:
    return;
  }
}

void midi_init() {
  in_position = 0;
  out_position = 0;
  out_size = 0;
  queue_init(&in, sizeof(midi_message_t), 32);
  queue_init(&out, sizeof(midi_message_t), OUT_MESSAGES_SIZE);
  uart_init(uart1, 31250);
  uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
  gpio_set_function(8, GPIO_FUNC_UART);
  gpio_set_function(9, GPIO_FUNC_UART);
}

void midi_run() {
  if(uart_is_readable(uart1)) {
    uart_read_blocking(uart1, in_buffer + in_position, 1);
    if(in_position == 2) {
      midi_message_t message;
      switch(in_buffer[0]) {
      case MIDI_NOTE_OFF:
        read_note(MIDI_NOTE_OFF_MESSAGE, &message);
        if(!queue_try_add(&in, &message)) {
          panic("MIDI in queue is full!");
        }
        in_position = 0;
        break;
      case MIDI_NOTE_ON:
        read_note(MIDI_NOTE_ON_MESSAGE, &message);
        if(!queue_try_add(&in, &message)) {
          panic("MIDI in queue is full!");
        }
        in_position = 0;
        break;
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
    midi_write_message(message);
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
  return OUT_MESSAGES_SIZE - queue_get_level(&out);
}

void midi_send_messages(midi_message_t * messages, const uint32_t messages_size) {
  for(uint32_t i = 0; i < messages_size; i++) {
    queue_add_blocking(&out, messages + i);
  }
}
