#pragma once
#include "pico/stdlib.h"

void i2c_controller_init();

bool i2c_controller_update_blocking(int32_t * const values, const int32_t * const max, const int32_t * const min);
