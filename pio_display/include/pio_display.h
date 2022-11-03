#pragma once
#include "pico/stdlib.h"


void pio_display_init();
void pio_display_update(const uint32_t * const data, const size_t size);
void pio_display_wait_for_finish_blocking();
