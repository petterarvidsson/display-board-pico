#include "pico/stdlib.h"
#include "stdio.h"
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

static uint dma_init(PIO pio, uint sm) {
  int channel = dma_claim_unused_channel(true);

  dma_channel_config channel_config = dma_channel_get_default_config(channel);
  channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
  dma_channel_configure(channel,
                        &channel_config,
                        &pio->txf[sm],
                        NULL,
                        0,
                        false);
  return channel;
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

  // Initialize shift register with 1s
  dma_channel_transfer_from_buffer_now(channel, shift40, sizeof(shift40) / sizeof(*shift40));
  dma_channel_wait_for_finish_blocking(channel);

  pio = pio0;

  uint offset = pio_add_program(pio, &spi_program);
  uint sm = pio_claim_unused_sm(pio, true);

  spi_program_init(pio, sm, offset, MOSI, DC, SCLK);

  channel = dma_init(pio, sm);
}

void pio_display_update(const uint32_t * const data, const size_t size) {
    // Activate first display
    gpio_put(CS, 0);
    dma_channel_transfer_from_buffer_now(channel, shift1, sizeof(shift1) / sizeof(*shift1));
    dma_channel_wait_for_finish_blocking(channel);

    // Push data to all displays
    gpio_put(CS, 1);
    dma_channel_transfer_from_buffer_now(channel, data, size);
}

void pio_display_wait_for_finish_blocking() {
  dma_channel_wait_for_finish_blocking(channel);
}