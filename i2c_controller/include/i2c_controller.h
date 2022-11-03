#pragma once
#include "pico/stdlib.h"

void i2c_controller_init();

int32_t i2c_controller_update_blocking();
