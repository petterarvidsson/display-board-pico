#pragma once
#include "pico/stdlib.h"

typedef enum {
  I2C_CONTROLLER_NO_CHANGE = -1, I2C_CONTROLLER_RELEASED = 0, I2C_CONTROLLER_PRESSED = 1
} i2c_controller_button_t;


void i2c_controller_init();

void i2c_controller_run();

bool i2c_controller_update(int32_t * const change_update, i2c_controller_button_t * const button);
