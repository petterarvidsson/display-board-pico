#pragma once
#include "pico/stdlib.h"

void i2c_controller_init();

void i2c_controller_run();

bool i2c_controller_update(int32_t * const change_update);
