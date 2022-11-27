#include "pico/stdlib.h"
#include "stdio.h"
#include <string.h>
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"
#include "pio_display.h"
#define SCLK 18
#define MOSI 19
#define CS 20
#define DC 21
#define SHIFT_CS 22
#define RESET 26

static PIO pio;
static uint channel;

static uint32_t shift1[] = {
  2
};

static uint32_t shift40[] = {
  2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2
};

static uint32_t initialize[] = {
  2 << 16, 0x8d14af81, 0xffe3e3e3
};

static uint32_t header[] = {
  1 << 16, 0xe3B00210,
  (32 << 16) | 1,
};

#define DISPLAY_ROW (32 * 4)
#define DISPLAY_ROW_HEADER (3 * 4)
#define DISPLAY_ROWS 8
#define DISPLAY_ROW_SIZE (DISPLAY_ROW + DISPLAY_ROW_HEADER)
#define DISPLAY_SIZE (DISPLAY_ROW_SIZE * DISPLAY_ROWS)
#define DISPLAYS 40
#define FRAMEBUFFER_SIZE  DISPLAY_SIZE * DISPLAYS
#define FRAMEBUFFER_SIZE_32 (FRAMEBUFFER_SIZE / 4)

static uint8_t framebuffer[FRAMEBUFFER_SIZE];

static uint dma_init(PIO pio, uint sm) {
  int channel = dma_claim_unused_channel(true);

  dma_channel_config channel_config = dma_channel_get_default_config(channel);
  channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
  channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
  dma_channel_configure(channel,
                        &channel_config,
                        &pio->txf[sm],
                        NULL,
                        0,
                        false);
  return channel;
}

uint8_t *pio_display_get(const uint8_t i) {
  return framebuffer + (i * DISPLAY_SIZE);
}

void pio_display_fill(uint8_t * const fb, const uint8_t pattern) {
  for(uint8_t i = 0; i < DISPLAY_ROWS; i++)
    memset(fb + i * DISPLAY_ROW_SIZE + DISPLAY_ROW_HEADER, pattern, DISPLAY_ROW);
}

void pio_display_clear(uint8_t * const fb) {
  pio_display_fill(fb, 0x00);
}

void pio_display_pixel(uint8_t * const fb, const uint8_t x, const uint8_t y, const bool on) {
    uint8_t real_y = y / 8;
    int pos = real_y * DISPLAY_ROW_SIZE + x + DISPLAY_ROW_HEADER;
    uint8_t seg = fb[pos];
    fb[pos] ^= (-on ^ seg) & (1 << (y % 8));
}

void pio_display_init() {
  gpio_init(RESET);
  gpio_set_dir(RESET, GPIO_OUT);
  gpio_put(RESET, 0);
  sleep_ms(1);
  gpio_put(RESET, 1);

  gpio_init(CS);
  gpio_set_dir(CS, GPIO_OUT);
  gpio_put(CS, 1);

  pio = pio0;

  uint offset = pio_add_program(pio, &spi_program);
  uint sm = pio_claim_unused_sm(pio, true);

  spi_program_init(pio, sm, offset, MOSI, DC, SCLK);

  channel = dma_init(pio, sm);

  // Initialize displays all at once
  dma_channel_transfer_from_buffer_now(channel, initialize, sizeof(initialize) / sizeof(*initialize));
  dma_channel_wait_for_finish_blocking(channel);

  // Initialize shift register with 1s
  dma_channel_transfer_from_buffer_now(channel, shift40, sizeof(shift40) / sizeof(*shift40));
  dma_channel_wait_for_finish_blocking(channel);

  for(uint8_t i = 0; i < DISPLAYS; i++) {
    uint8_t *display = pio_display_get(i);
    for(uint8_t j = 0; j < DISPLAY_ROWS; j++) {
      memcpy(display + j * DISPLAY_ROW_SIZE, header, DISPLAY_ROW_HEADER);
      if(j == 0)
        display[0] = 0x02;
      display[j * DISPLAY_ROW_SIZE + 6] = 0xB0 + j;
    }
    pio_display_fill(display, 0x00);
  }
  pio_display_update();
  pio_display_wait_for_finish_blocking();
}

void pio_display_update() {
  // Activate first display
  gpio_put(CS, 0);
  dma_channel_transfer_from_buffer_now(channel, shift1, sizeof(shift1) / sizeof(*shift1));
  dma_channel_wait_for_finish_blocking(channel);

  // We need to wait for PIO to send the clock pulse to shift in the first bit
  busy_wait_us_32(10);
  gpio_put(CS, 1);

  // Push data to all displays
  dma_channel_transfer_from_buffer_now(channel, framebuffer, FRAMEBUFFER_SIZE_32);
}



void pio_display_wait_for_finish_blocking() {
  dma_channel_wait_for_finish_blocking(channel);
}

bool pio_display_can_wait_without_blocking() {
  return !dma_channel_is_busy(channel);
}
