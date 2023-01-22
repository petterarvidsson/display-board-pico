#pragma once
#include "pico/util/queue.h"
#include "stdint.h"
#include "stdbool.h"

typedef enum {
  MIDI_NOTE_ON_MESSAGE,
  MIDI_NOTE_OFF_MESSAGE,
  MIDI_RAW_MESSAGE
} midi_message_type_t;

typedef struct {
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
} note_message_t;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t z;
} raw_message_t;

typedef union {
  raw_message_t raw;
  note_message_t note;
} midi_message_value_t;

typedef struct {
  midi_message_type_t type;
  midi_message_value_t value;
} midi_message_t;

void midi_init();
void midi_run();
uint32_t midi_get_available_messages(midi_message_t * messages, const uint32_t messages_size);
uint32_t midi_can_send_messages();
void midi_send_messages(midi_message_t * messages, const uint32_t messages_size);
void midi_set_mapped_note(const uint8_t note, const uint8_t out_channel, const uint8_t out_note);
void midi_clear_mapped_note(const uint8_t note);
