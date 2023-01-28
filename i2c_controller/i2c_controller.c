#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/mutex.h"
#include "hardware/i2c.h"
#include "i2c_controller.h"
#define I2C_SCL 5
#define I2C_SDA 4
#define CONTROLLERS 9


static i2c_inst_t *i2c;
static const int addr0 = 0x20;
static const int addr1 = 0x21;
static const uint8_t reg0 = 0;
// Pin connections for pin A, B and D
// A   B   D
static const uint8_t controller_connections[CONTROLLERS][3] = {
  {3, 4, 5}, // E1 0xFFFFFFDF
  {10,  11,   12}, // E2 0xFFFFEFFF
  {13,  14,   15}, // E3 0xFFFF7FFF
  {0,  1,  2}, // E4 0xFFFFFFFB
  {24, 25, 26}, // E5 0xFBFFFFFF
  {27, 28, 29}, // E6 0xDFFFFFFF
  {21, 22, 23}, // E7 0xFF7FFFFF
  {18, 19, 20}, // E8 0xFFEFFFFF
  {30, 31, 17}  // E9 0x3FFDFFFF
};
static uint8_t old_a[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1
};

static uint32_t rxdata;

static int32_t change[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0
};

auto_init_mutex(mutex);

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

void i2c_controller_run() {
  i2c_write_blocking(i2c_default, addr0, &reg0, 1, true);
  i2c_read_blocking(i2c_default, addr0, (uint8_t*)&rxdata, 2, false);
  i2c_write_blocking(i2c_default, addr1, &reg0, 1, true);
  i2c_read_blocking(i2c_default, addr1, ((uint8_t*)&rxdata + 2), 2, false);


  mutex_enter_blocking(&mutex);
  for(uint8_t i = 0; i < CONTROLLERS; i++) {
    uint8_t a = (rxdata >> controller_connections[i][0]) & 0x1;
    uint8_t b = (rxdata >> controller_connections[i][1]) & 0x1;
    if(a == 1 && a != old_a[i]) {
      if(b) {
        change[i]--;
      } else {
        change[i]++;
      }
    }
    old_a[i] = a;
  }
  mutex_exit(&mutex);
}

bool i2c_controller_update(int32_t * const change_update) {
  bool changed = false;

  mutex_enter_blocking(&mutex);
  for(uint8_t i = 0; i < CONTROLLERS; i++) {
    if(change[i] != 0) {
      change_update[i] = change[i];
      change[i] = 0;
      changed = true;
    }
  }
  mutex_exit(&mutex);

  return changed;
}
