#pragma once
#include "pico/util/queue.h"
#include "stdint.h"
#include "stdbool.h"

typedef enum {
  MIDI_CONTROLLER_MESSAGE,
  MIDI_NOTE_ON_MESSAGE,
  MIDI_NOTE_OFF_MESSAGE,
  MIDI_PROGRAM_CHANGE_MESSAGE,
  MIDI_RPN_MESSAGE,
  MIDI_NRPN_MESSAGE,
  MIDI_RAW_MESSAGE
} midi_message_type_t;

typedef struct {
    uint8_t channel;
    uint8_t number;
    uint8_t value;
} controller_message_t;

typedef struct {
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
} note_message_t;

typedef struct {
    uint8_t channel;
    uint8_t number;
} program_message_t;

typedef struct {
  uint8_t channel;
  uint8_t msb;
  uint8_t lsb;
  uint8_t value;
} rpn_message_t;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t z;
} raw_message_t;

typedef union {
  controller_message_t controller;
  raw_message_t raw;
  program_message_t program;
  note_message_t note;
  rpn_message_t rpn;
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
void midi_send_bank_change(const uint8_t channel, const uint8_t bank);
void midi_send_volume(const uint8_t channel, const uint8_t volume);
void midi_send_attack(const uint8_t channel, const uint8_t attack);
void midi_send_decay(const uint8_t channel, const uint8_t decay);
void midi_send_release(const uint8_t channel, const uint8_t release);
void midi_send_hpf_cutoff(const uint8_t channel, const uint8_t cutoff);
void midi_send_lpf_cutoff(const uint8_t channel, const uint8_t cutoff);
void midi_send_lpf_resonance(const uint8_t channel, const uint8_t resonance);
