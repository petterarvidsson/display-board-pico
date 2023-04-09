#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define YM3812_NUM_CHANNELS  9
#define YM3812_NUM_OPERATORS 18

static i2c_inst_t *i2c;

#define I2C_SCL 3
#define I2C_SDA 2

#define GPIO_ADDR 0x20

// NC | NC | NC | CS | WR | A1 | A0 | IC
#define RESET 0xFE
#define WRITE_ADDRESS_PORT0_PRE 0xF1
#define WRITE_ADDRESS_PORT0 0xE1
#define WRITE_ADDRESS_PORT1_PRE 0xF5
#define WRITE_ADDRESS_PORT1 0xE5
#define WRITE_DATA_PRE 0xF7
#define WRITE_DATA 0xE7
#define INACTIVE 0xFF

#define BUFFER_SIZE 16
static uint8_t i2c_buffer[BUFFER_SIZE];

static void write(uint8_t a1, uint8_t reg, uint8_t data) {
  //printf("PORT: %d REG %02X: %02X\n", a1, reg, data);

  // Write to data port
  i2c_buffer[0] = 2;
  if(a1 == 0) {
    i2c_buffer[1] = WRITE_ADDRESS_PORT0_PRE;
  } else {
    i2c_buffer[1] = WRITE_ADDRESS_PORT1_PRE;
  }
  i2c_buffer[2] = reg;
  if(a1 == 0) {
    i2c_buffer[3] = WRITE_ADDRESS_PORT0;
  } else {
    i2c_buffer[3] = WRITE_ADDRESS_PORT1;
  }
  i2c_buffer[4] = reg;
  if(a1 == 0) {
    i2c_buffer[5] = WRITE_ADDRESS_PORT0_PRE;
  } else {
    i2c_buffer[5] = WRITE_ADDRESS_PORT1_PRE;
  }
  i2c_buffer[6] = reg;
  i2c_buffer[7] = INACTIVE;
  i2c_buffer[8] = data;
  i2c_buffer[9] = WRITE_DATA_PRE;
  i2c_buffer[10] = data;
  i2c_buffer[11] = WRITE_DATA;
  i2c_buffer[12] = data;
  i2c_buffer[13] = WRITE_DATA_PRE;
  i2c_buffer[14] = data;
  i2c_buffer[15] = INACTIVE;
  int error = i2c_write_blocking(i2c, GPIO_ADDR, i2c_buffer, BUFFER_SIZE, false);
  if(error <= 0) {
    printf("%d ERROR\n", error);
  }

}

void i2c_controller_init() {
  i2c = i2c1;
  i2c_init(i2c, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));

  int error;

  // Set output registers state INACTIVE and data 00
  const uint8_t initialize_out[] = {2, INACTIVE, 0x00};
  error = i2c_write_blocking(i2c, GPIO_ADDR, initialize_out, sizeof(initialize_out) / sizeof(uint8_t), false);
  if(error <= 0) {
    printf("%d ERROR\n", error);
  }

  // Activate both output ports
  const uint8_t activate_out[] = {6, 0x00, 0x00};
  error = i2c_write_blocking(i2c, GPIO_ADDR, activate_out, sizeof(activate_out) / sizeof(uint8_t), false);
  if(error <= 0) {
    printf("%d ERROR\n", error);
  }
}

void ymf262_init() {
  i2c_controller_init();
  printf("Hello ymf262\n");

  const uint8_t reset[] = {
    2,
    RESET
  };

  const uint8_t inactive[] = {
    2,
    INACTIVE
  };

  busy_wait_ms(10);
  i2c_write_blocking(i2c, GPIO_ADDR, reset, sizeof(reset) / sizeof(uint8_t), false);
  busy_wait_ms(10);
  i2c_write_blocking(i2c, GPIO_ADDR, inactive, sizeof(inactive) / sizeof(uint8_t), false);
  busy_wait_ms(10);

  uint8_t channel_map[YM3812_NUM_CHANNELS] = { 0,1,2,6,7,8,12,13,14 };
  uint8_t op_map[YM3812_NUM_OPERATORS]     = { 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21 };
  for( uint8_t ch=0; ch<YM3812_NUM_CHANNELS; ch++){   // Use the same patch for all channels
    uint8_t op1_index = channel_map[ch];
    uint8_t op2_index = op1_index + 3;                        // Always 3 higher
    uint8_t op1 = op_map[op1_index];
    uint8_t op2 = op_map[op2_index];
    //Channel settings
    write(0, 0xC0 + ch, 1); // Algorithm (Addative synthesis) + Feedbacl 0

    //Operator 1's settings
    write(0, 0x60 + op1, 0xB6); // Attack + decay
    write(0, 0x80 + op1, 0xA2); // Sustain + release
    write(0, 0x40 + op1, 0x00);
    //write(0, 0xE0 + op1, 0x01);

    //Operator 1's settings
    write(0, 0x60 + op2, 0xB6); // Attack + decay
    write(0, 0x80 + op2, 0xA2); // Sustain + release
    write(0, 0x40 + op2, 0x00);
    //write(0, 0xE0 + op2, 0x01);
  }

  write(0, 0xB0 + 0, 0);
  write(0, 0xB0 + 1, 0);
  write(0, 0xB0 + 2, 0);
  write(0, 0xB0 + 3, 0);

  while(true) {

  write(0, 0xB0 + 0, (4 << 2) | (0x1C9 >> 8)); // Block 4 + freq H
  write(0, 0xA0 + 0, 0x1C9 & 0xFF); // Freq L

  write(0, 0xB0 + 1, (4 << 2) | (0x240 >> 8)); // Block 4 + freq H
  write(0, 0xA0 + 1, 0x1C9 & 0xFF); // Freq L

  write(0, 0xB0 + 2, (4 << 2) | (0x2AD >> 8)); // Block 4 + freq H
  write(0, 0xA0 + 2, 0x1C9 & 0xFF); // Freq L

  write(0, 0xB0 + 3, (4 << 2) | (0x360 >> 8)); // Block 4 + freq H
  write(0, 0xA0 + 3, 0x1C9 & 0xFF); // Freq L

  write(0, 0xB0 + 0, (1 << 5) | (4 << 2) | (0x1C9 >> 8)); // Block 4 + freq H + key ON
  busy_wait_ms(200);

  write(0, 0xB0 + 1, (1 << 5) | (4 << 2) | (0x240 >> 8)); // Block 4 + freq H + key ON
  busy_wait_ms(200);

  write(0, 0xB0 + 2, (1 << 5) | (4 << 2) | (0x2AD >> 8)); // Block 4 + freq H + key ON
  busy_wait_ms(200);

  write(0, 0xB0 + 3, (1 << 5) | (4 << 2) | (0x360 >> 8)); // Block 4 + freq H + key ON
  busy_wait_ms(200);

  write(0, 0xB0 + 0, (4 << 2) | (0x1C9 >> 8)); // Block 4 + freq H
  write(0, 0xB0 + 1, (4 << 2) | (0x240 >> 8)); // Block 4 + freq H
  write(0, 0xB0 + 2, (4 << 2) | (0x2AD >> 8)); // Block 4 + freq H
  write(0, 0xB0 + 3, (4 << 2) | (0x360 >> 8)); // Block 4 + freq H
  }
  /*
  busy_wait_ms(2000);
  i2c_write_blocking(i2c, GPIO_ADDR, reset, sizeof(reset) / sizeof(uint8_t), false);
  busy_wait_ms(10);
  i2c_write_blocking(i2c, GPIO_ADDR, inactive, sizeof(inactive) / sizeof(uint8_t), false);
  busy_wait_ms(10);
  */

}
