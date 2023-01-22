#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/platform.h"
#include "midi.h"

static uint8_t buffer[3];
static uint8_t position;
static queue_t messages;

static void read_note(uint8_t *data, midi_message_type_t type, midi_message_t *midi) {
    note_message_t midi_note = {
        data[0] & 0x0F,
        data[1] & 0x7F,
        data[2] & 0x7F
    };
    midi->type = type;
    midi->value.note = midi_note;
}

static void read_raw(uint8_t *data, midi_message_t *midi) {
    raw_message_t raw = {data[0], data[1], data[2]};
    midi->type = MIDI_RAW_MESSAGE;
    midi->value.raw = raw;
}

queue_t * midi_init() {
  position = 0;
  queue_init(&messages, sizeof(midi_message_t), 32);
  uart_init(uart1, 31250);
  gpio_set_function(8, GPIO_FUNC_UART);
  gpio_set_function(9, GPIO_FUNC_UART);
  return &messages;
}

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80

void midi_run() {
  if(uart_is_readable(uart1)) {
    uart_read_blocking(uart1, buffer + position, 1);
    position++;
    if(position == 2) {
      midi_message_t message;
      switch(buffer[0]) {
      case MIDI_NOTE_OFF:
        read_note(buffer, MIDI_NOTE_OFF_MESSAGE, &message);
        break;
      case MIDI_NOTE_ON:
        read_note(buffer, MIDI_NOTE_ON_MESSAGE, &message);
        break;
      default:
        read_raw(buffer, &message);
        break;
      }

      if(!queue_try_add(&messages, &message)) {
        panic("MIDI queue is full!");
      }
      position = 0;
    }
  }
}

uint32_t midi_get_available_messages(midi_message_t * available, const uint32_t available_size) {
  for(uint32_t i = 0; i < available_size; i++) {
    if(!queue_try_remove(&messages, available + i)) {
      return i;
    }
  }
}
