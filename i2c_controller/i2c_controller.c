#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c_controller.h"
#define I2C_SCL 5
#define I2C_SDA 4

static i2c_inst_t *i2c;
static const int addr = 0x20;
static const uint8_t reg0 = 0;
static int32_t value = 0;
static uint8_t old_a = 1;
static uint8_t rxdata[] = {0,0};

void i2c_controller_init() {
  i2c = i2c0;
  i2c_init(i2c, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
}

int32_t i2c_controller_update_blocking() {
  int wret = i2c_write_blocking(i2c_default, addr, &reg0, 1, true);
  int ret = i2c_read_blocking(i2c_default, addr, rxdata, 2, false);
  uint8_t a = (rxdata[1] >> 3) & 0x1;
  uint8_t b = (rxdata[1] >> 4) & 0x1;
  if(a == 1 && a != old_a) {
    if(b) {
      value--;
    } else {
      value++;
    }
  }
  old_a = a;
  return value;
}
