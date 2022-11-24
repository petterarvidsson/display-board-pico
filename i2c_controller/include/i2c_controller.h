#pragma once
#include "pico/stdlib.h"

void i2c_controller_init();

uint8_t i2c_controller_update_blocking(int32_t *values);
