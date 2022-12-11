#pragma once
#include "pico/stdlib.h"

void i2c_controller_init();

void i2c_controller_run_loop();
bool i2c_controller_update(int32_t * const values, const int32_t * const max, const int32_t * const min);
