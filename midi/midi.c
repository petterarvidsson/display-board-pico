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

static void write_size(uint8_t size) {
  out_size = size;
  out_position = 0;
}

static void midi_write_note(const note_message_t note_on, uint8_t status_prefix) {
  out_buffer[0] = status_prefix + (note_on.channel & 0x0F);
  out_buffer[1] = note_on.note & 0x7F;
  out_buffer[2] = note_on.velocity &0x7F;
  write_size(3);
}

static void midi_write_controller(const controller_message_t controller) {
  out_buffer[0] = 0xB0 + (controller.channel & 0x0F);
  out_buffer[1] = controller.number & 0x7F;
  out_buffer[2] = controller.value & 0x7F;
  write_size(3);
}

static void midi_write_program_change(const program_message_t program) {
  out_buffer[0] = 0xC0 + (program.channel & 0x0F);
  out_buffer[1] = program.number & 0x7F;
  write_size(2);
}

static void midi_write_raw(const raw_message_t raw) {
  out_buffer[0] = raw.x;
  out_buffer[1] = raw.y;
  out_buffer[2] = raw.z;
  write_size(3);
}

void midi_write_message(const midi_message_t message) {
  switch(message.type) {
  case MIDI_CONTROLLER_MESSAGE:
    midi_write_controller(message.value.controller);
    return;
  case MIDI_NOTE_ON_MESSAGE:
    midi_write_note(message.value.note, MIDI_NOTE_ON);
    return;
  case MIDI_NOTE_OFF_MESSAGE:
    midi_write_note(message.value.note, MIDI_NOTE_OFF);
    return;
  case MIDI_PROGRAM_CHANGE_MESSAGE:
    midi_write_program_change(message.value.program);
    return;
  case MIDI_RAW_MESSAGE:
    midi_write_raw(message.value.raw);
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
        printf("%d %d\n", message.value.note.note, map.note);
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

void midi_send_bank_change(const uint8_t channel, const uint8_t bank) {
  midi_message_t messages[] = {
    {
      .type = MIDI_CONTROLLER_MESSAGE,
      .value.controller = {
        .channel = channel & 0x0F,
        .number = 0,
        .value = 127
      }
    },
    {
      .type = MIDI_CONTROLLER_MESSAGE,
      .value.controller = {
        .channel = channel & 0x0F,
        .number = 32,
        .value = 0
      }
    },
    {
      .type = MIDI_PROGRAM_CHANGE_MESSAGE,
      .value.controller = {
        .channel = channel & 0x0F,
        .number = bank
      }
    }
  };
  midi_send_messages(messages, sizeof(messages) / sizeof(midi_message_t));
}

void midi_send_volume(const uint8_t channel, const uint8_t volume) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x0F,
      .number = 7,
      .value = volume & 0x7F
    }
  };
  queue_add_blocking(&out, &message);
}

void midi_send_attack(const uint8_t channel, const uint8_t attack) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x0F,
      .number = 73,
      .value = attack & 0x7F
    }
  };
  queue_add_blocking(&out, &message);
}

void midi_send_decay(const uint8_t channel, const uint8_t decay) {
}


void midi_send_release(const uint8_t channel, const uint8_t release) {
  midi_message_t message = {
    .type = MIDI_CONTROLLER_MESSAGE,
    .value.controller = {
      .channel = channel & 0x0F,
      .number = 72,
      .value = release & 0x7F
    }
  };
  queue_add_blocking(&out, &message);
}
