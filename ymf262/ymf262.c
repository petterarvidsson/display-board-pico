#include <math.h>
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ymf262.h"
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

#define M_FREQ 14318210
#define S_FREQ ((float)(M_FREQ) / 288.0f)
#define B_FREQ(BLOCK) (S_FREQ / (float)(1 << (20 - BLOCK)))
#define B_FREQ_MAX(BLOCK) (B_FREQ(BLOCK) * (float)0x3FF)

#define REGISTERS 0xF6

static uint8_t a[2][REGISTERS];

typedef struct {
  uint8_t f_num_l;
  uint8_t f_num_h;
  uint8_t block;
} f_number_t;

typedef enum {
  GLOBAL,
  CHANNEL,
  SLOT_1,
  SLOT_2,
  SLOT_3,
  SLOT_4
} parameter_type_t;

typedef struct {
  parameter_type_t type;
  uint8_t base_reg;
  uint8_t mask;
  uint8_t shift;
} parameter_offset_t;

parameter_offset_t parameter_offsets[] = {
  { // CONN_SEL
    .type = GLOBAL,
    .base_reg = 0x04,
    .mask = 0x1,
    .shift = 0 // CHANNEL is used
  },
  { // AM_1
    .type = SLOT_1,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 7
  },
  { // AM_2
    .type = SLOT_2,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 7
  },
  { // AM_3
    .type = SLOT_3,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 7
  },
  { // AM_4
    .type = SLOT_4,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 7
  },
  { // VIB_1
    .type = SLOT_1,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 6
  },
  { // VIB_2
    .type = SLOT_2,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 6
  },
  { // VIB_3
    .type = SLOT_3,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 6
  },
  { // VIB_4
    .type = SLOT_4,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 6
  },
  { // EGT_1
    .type = SLOT_1,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 5
  },
  { // EGT_2
    .type = SLOT_2,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 5
  },
  { // EGT_3
    .type = SLOT_3,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 5
  },
  { // EGT_4
    .type = SLOT_4,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 5
  },
  { // KSR_1
    .type = SLOT_1,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 4
  },
  { // KSR_2
    .type = SLOT_2,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 4
  },
  { // KSR_3
    .type = SLOT_3,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 4
  },
  { // KSR_4
    .type = SLOT_4,
    .base_reg = 0x20,
    .mask = 0x1,
    .shift = 4
  },
  { // MULT_1
    .type = SLOT_1,
    .base_reg = 0x20,
    .mask = 0xF,
    .shift = 0
  },
  { // MULT_2
    .type = SLOT_2,
    .base_reg = 0x20,
    .mask = 0xF,
    .shift = 0
  },
  { // MULT_3
    .type = SLOT_3,
    .base_reg = 0x20,
    .mask = 0xF,
    .shift = 0
  },
  { // MULT_4
    .type = SLOT_4,
    .base_reg = 0x20,
    .mask = 0xF,
    .shift = 0
  },
  { // KSL_1
    .type = SLOT_1,
    .base_reg = 0x40,
    .mask = 0x3,
    .shift = 6
  },
  { // KSL_2
    .type = SLOT_2,
    .base_reg = 0x40,
    .mask = 0x3,
    .shift = 6
  },
  { // KSL_3
    .type = SLOT_3,
    .base_reg = 0x40,
    .mask = 0x3,
    .shift = 6
  },
  { // KSL_4
    .type = SLOT_4,
    .base_reg = 0x40,
    .mask = 0x3,
    .shift = 6
  },
  { // TL_1
    .type = SLOT_1,
    .base_reg = 0x40,
    .mask = 0x3F,
    .shift = 0
  },
  { // TL_2
    .type = SLOT_2,
    .base_reg = 0x40,
    .mask = 0x3F,
    .shift = 0
  },
  { // TL_3
    .type = SLOT_3,
    .base_reg = 0x40,
    .mask = 0x3F,
    .shift = 0
  },
  { // TL_4
    .type = SLOT_4,
    .base_reg = 0x40,
    .mask = 0x3F,
    .shift = 0
  },
  { // AR_1
    .type = SLOT_1,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 4
  },
  { // AR_2
    .type = SLOT_2,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 4
  },
  { // AR_3
    .type = SLOT_3,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 4
  },
  { // AR_4
    .type = SLOT_4,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 4
  },
  { // DR_1
    .type = SLOT_1,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 0
  },
  { // DR_2
    .type = SLOT_2,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 0
  },
  { // DR_3
    .type = SLOT_3,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 0
  },
  { // DR_4
    .type = SLOT_4,
    .base_reg = 0x60,
    .mask = 0xF,
    .shift = 0
  },
  { // SL_1
    .type = SLOT_1,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 4
  },
  { // SL_2
    .type = SLOT_2,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 4
  },
  { // SL_3
    .type = SLOT_3,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 4
  },
  { // SL_4
    .type = SLOT_4,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 4
  },
  { // RR_1
    .type = SLOT_1,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 0
  },
  { // RR_2
    .type = SLOT_2,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 0
  },
  { // RR_3
    .type = SLOT_3,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 0
  },
  { // RR_4
    .type = SLOT_4,
    .base_reg = 0x80,
    .mask = 0xF,
    .shift = 0
  },
  { // FNUM_L
    .type = CHANNEL,
    .base_reg = 0xA0,
    .mask = 0xFF,
    .shift = 0
  },
  { // KON
    .type = CHANNEL,
    .base_reg = 0xB0,
    .mask = 0x1,
    .shift = 5
  },
  { // BLOCK
    .type = CHANNEL,
    .base_reg = 0xB0,
    .mask = 0x7,
    .shift = 2
  },
  { // FNUM_H
    .type = CHANNEL,
    .base_reg = 0xB0,
    .mask = 0x3,
    .shift = 0
  },
  { // CHD
    .type = CHANNEL,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 7
  },
  { // CHC
    .type = CHANNEL,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 6
  },
  { // CHB
    .type = CHANNEL,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 5
  },
  { // CHA
    .type = CHANNEL,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 4
  },
  { // FB
    .type = CHANNEL,
    .base_reg = 0xC0,
    .mask = 0x7,
    .shift = 1
  },
  { // CNT_1
    .type = SLOT_1,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 0
  },
  { // CNT_3
    .type = SLOT_2,
    .base_reg = 0xC0,
    .mask = 0x1,
    .shift = 0
  },
  { // WS_1
    .type = SLOT_1,
    .base_reg = 0xE0,
    .mask = 0x7,
    .shift = 0
  },
  { // WS_2
    .type = SLOT_2,
    .base_reg = 0xE0,
    .mask = 0x7,
    .shift = 0
  },
  { // WS_3
    .type = SLOT_3,
    .base_reg = 0xE0,
    .mask = 0x7,
    .shift = 0
  },
  { // WS_4
    .type = SLOT_4,
    .base_reg = 0xE0,
    .mask = 0x7,
    .shift = 0
  }
};

#define BUFFER_SIZE 12
static uint8_t i2c_buffer[BUFFER_SIZE];

static void write(uint8_t a1, uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data) {
  uint8_t new = ((data & mask) << shift) | (a[a1 & 0x1][reg] & ~(mask << shift));
  printf("PORT: %d REG %02X: %02X\n", a1, reg, new);

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
  i2c_buffer[5] = INACTIVE;
  i2c_buffer[6] = new;
  i2c_buffer[7] = WRITE_DATA_PRE;
  i2c_buffer[8] = new;
  i2c_buffer[9] = WRITE_DATA;
  i2c_buffer[10] = new;
  i2c_buffer[11] = INACTIVE;
  i2c_write_blocking(i2c, GPIO_ADDR, i2c_buffer, BUFFER_SIZE, false);
  a[a1 & 0x1][reg] = new;
}

static f_number_t f_number(const float freq) {
  uint8_t block = 7;
  if(freq < B_FREQ_MAX(0)) {
    block = 0;
  } else if(freq < B_FREQ_MAX(1)) {
    block = 1;
  } else if(freq < B_FREQ_MAX(2)) {
    block = 2;
  } else if(freq < B_FREQ_MAX(3)) {
    block = 3;
  } else if(freq < B_FREQ_MAX(4)) {
    block = 4;
  } else if(freq < B_FREQ_MAX(5)) {
    block = 5;
  } else if(freq < B_FREQ_MAX(6)) {
    block = 6;
  }

  uint16_t f_num = round(freq / B_FREQ(block));

  f_number_t frequency = {
    f_num & 0xFF,
    (f_num >> 8) & 0x3,
    block
  };
  return frequency;
}

typedef struct {
  uint8_t a1;
  uint8_t addr;
} reg_t;

static uint8_t slot_to_reg[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15};
static uint8_t channel_slot1_op[] = {1, 2, 3, 19, 20, 21};
static uint8_t channel_slot2_op[] = {4, 5, 6, 22, 23, 24};
static uint8_t channel_slot3_op[] = {7, 8, 9, 25, 26, 27};
static uint8_t channel_slot4_op[] = {10, 11, 12, 28, 29, 30};

#define SLOTS_PER_PORT 18
#define CHANNELS_PER_PORT 3
#define CHANNELS (CHANNELS_PER_PORT * 2)

static inline reg_t c_s1_reg(const uint8_t c, const uint8_t r) {
  reg_t reg = {
    (channel_slot1_op[c] - 1) / SLOTS_PER_PORT,
    r + slot_to_reg[(channel_slot1_op[c] - 1) % SLOTS_PER_PORT]
  };
  return reg;
}

static inline reg_t c_s2_reg(const uint8_t c, const uint8_t r) {
  reg_t reg = {
    c / CHANNELS_PER_PORT,
    r + slot_to_reg[(channel_slot2_op[c] - 1) % SLOTS_PER_PORT]
  };
  return reg;
}

static inline reg_t c_s3_reg(const uint8_t c, const uint8_t r) {
  reg_t reg = {
    c / CHANNELS_PER_PORT,
    r + slot_to_reg[(channel_slot3_op[c] - 1) % SLOTS_PER_PORT]
  };
  return reg;
}

static inline reg_t c_s4_reg(const uint8_t c, const uint8_t r) {
  reg_t reg = {
    c / CHANNELS_PER_PORT,
    r + slot_to_reg[(channel_slot4_op[c] - 1) % SLOTS_PER_PORT]
  };
  return reg;
}

static void write_s1(const uint8_t c, const uint8_t r, const uint8_t m, const uint8_t s, const uint8_t data) {
  reg_t reg = c_s1_reg(c, r);
  write(reg.a1, reg.addr, m, s, data);
}

static void write_s2(const uint8_t c, const uint8_t r, const uint8_t m, const uint8_t s, const uint8_t data) {
  reg_t reg = c_s2_reg(c, r);
  write(reg.a1, reg.addr, m, s, data);
}

static void write_s3(const uint8_t c, const uint8_t r, const uint8_t m, const uint8_t s, const uint8_t data) {
  reg_t reg = c_s3_reg(c, r);
  write(reg.a1, reg.addr, m, s, data);
}

static void write_s4(const uint8_t c, const uint8_t r, const uint8_t m, const uint8_t s, const uint8_t data) {
  reg_t reg = c_s4_reg(c, r);
  write(reg.a1, reg.addr, m, s, data);
}

static void write_c(const uint8_t c, const uint8_t r, const uint8_t m, const uint8_t s, const uint8_t data) {
  write(c / CHANNELS_PER_PORT, r + (c % CHANNELS_PER_PORT), m, s, data);
}

void ymf262_parameter(uint8_t c, ymf262_parameter_t parameter, uint8_t value) {
  parameter_offset_t offset = parameter_offsets[parameter];
  switch(offset.type) {
  case SLOT_1:
    write_s1(c, offset.base_reg, offset.mask, offset.shift, value);
    break;
  case SLOT_2:
    write_s2(c, offset.base_reg, offset.mask, offset.shift, value);
    break;
  case SLOT_3:
    write_s3(c, offset.base_reg, offset.mask, offset.shift, value);
    break;
  case SLOT_4:
    write_s4(c, offset.base_reg, offset.mask, offset.shift, value);
    break;
  case CHANNEL:
    write_c(c, offset.base_reg, offset.mask, offset.shift, value);
    break;
  case GLOBAL:
    if(parameter == CONN_SEL) {
      write(1, offset.base_reg, offset.mask, c, value);
    }
    break;
  }
}

void ymf262_all_channels_parameter(ymf262_parameter_t parameter, uint8_t value) {
  for(uint32_t c = 0; c < CHANNELS; c++) {
    ymf262_parameter(c, parameter, value);
  }
}

void ymf262_parameters(uint8_t c, const ymf262_parameter_value_t * const parameter_values, const uint32_t size) {
  for(uint32_t i = 0; i < size; i++) {
    const ymf262_parameter_value_t parameter_value = parameter_values[i];
    ymf262_parameter(c, parameter_value.parameter, parameter_value.value);
  }
}

#define CONNECTIONS 6
#define CONNECTION_PARAMETERS 3

ymf262_parameter_value_t connections[CONNECTIONS][CONNECTION_PARAMETERS] = {
  { // FM
    {CONN_SEL, 0},
    {CNT_1, 0},
    {CNT_3, 0}
  },
  { // AM
    {CONN_SEL, 0},
    {CNT_1, 1},
    {CNT_3, 0}
  },
  { // FMFM
    {CONN_SEL, 1},
    {CNT_1, 0},
    {CNT_3, 0}
  },
  { // FM_FM
    {CONN_SEL, 1},
    {CNT_1, 0},
    {CNT_3, 1}
  },
  { // AM_FM
    {CONN_SEL, 1},
    {CNT_1, 1},
    {CNT_3, 0}
  },
  { // AM_FM_AM
    {CONN_SEL, 1},
    {CNT_1, 1},
    {CNT_3, 1}
  }
};

void ymf262_channel_connection(uint8_t c, ymf262_channel_connection_t type) {
  ymf262_parameters(c, connections[type], CONNECTION_PARAMETERS);
}

void ymf262_start(uint8_t c) {
  ymf262_parameter(c, KON, 1);
}

void ymf262_frequency(uint8_t c, float frequency) {
  f_number_t f = f_number(frequency);
  ymf262_parameter_value_t parameter_values[] = {
    {FNUM_L, f.f_num_l},
    {FNUM_H, f.f_num_h},
    {BLOCK, f.block}
  };
  ymf262_parameters(c, parameter_values, sizeof(parameter_values) / sizeof(ymf262_parameter_value_t));
}

void ymf262_stop(uint8_t c) {
  ymf262_parameter(c, KON, 0);
}

static void i2c_controller_init() {
  i2c = i2c1;
  i2c_init(i2c, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));

  // Set output registers state INACTIVE and data 00
  const uint8_t initialize_out[] = {2, INACTIVE, 0x00};
  i2c_write_blocking(i2c, GPIO_ADDR, initialize_out, sizeof(initialize_out) / sizeof(uint8_t), false);

  // Activate both output ports
  const uint8_t activate_out[] = {6, 0x00, 0x00};
  i2c_write_blocking(i2c, GPIO_ADDR, activate_out, sizeof(activate_out) / sizeof(uint8_t), false);

}

#define YM3812_NUM_CHANNELS  9
#define YM3812_NUM_OPERATORS 18


void ymf262_init() {
  i2c_controller_init();

  const uint8_t reset[] = {
    2,
    RESET
  };

  const uint8_t inactive[] = {
    2,
    INACTIVE
  };

  i2c_write_blocking(i2c, GPIO_ADDR, reset, sizeof(reset) / sizeof(uint8_t), false);
  busy_wait_ms(10);
  i2c_write_blocking(i2c, GPIO_ADDR, inactive, sizeof(inactive) / sizeof(uint8_t), false);
  busy_wait_ms(10);

  // All registers start 0
  memset(a[0], 0, sizeof(uint8_t) * REGISTERS);
  memset(a[1], 0, sizeof(uint8_t) * REGISTERS);

  write(0, 0x01, 0xFF, 0, 0x00);
  write(0, 0x08, 0xFF, 0, 0x40);
  write(0, 0xBD, 0xFF, 0, 0x00);
  write(1, 0x04, 0xFF, 0, 0x00);
  // OPL3 mode enabled
  //write(1, 0x05, 0xFF, 0, 0x01);

  uint8_t channel_map[YM3812_NUM_CHANNELS] = { 0,1,2,6,7,8,12,13,14 };
  uint8_t op_map[YM3812_NUM_OPERATORS]     = { 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21 };
  for( uint8_t ch=0; ch<YM3812_NUM_CHANNELS; ch++){   // Use the same patch for all channels
    uint8_t op1_index = channel_map[ch];
    uint8_t op2_index = op1_index + 3;                        // Always 3 higher
    uint8_t op1 = op_map[op1_index];
    uint8_t op2 = op_map[op2_index];
    //Channel settings
    write(0, 0xC0 + ch, 0xFF, 0, 1); // Algorithm (Addative synthesis) + Feedbacl 0

    //Operator 1's settings
    write(0, 0x60 + op1, 0xFF, 0, 0x86); // Attack + decay
    write(0, 0x80 + op1, 0xFF, 0, 0xAA); // Sustain + release
    write(0, 0x40 + op1, 0xFF, 0, 0x00);
    //write(0, 0xE0 + op1, 0x01);

    //Operator 1's settings
    write(0, 0x60 + op2, 0xFF, 0, 0x86); // Attack + decay
    write(0, 0x80 + op2, 0xFF, 0, 0xAA); // Sustain + release
    write(0, 0x40 + op2, 0xFF, 0, 0x00);
    //write(0, 0xE0 + op2, 0x01);
  }

  write(0, 0xB0 + 0, 0xFF, 0, 0);
  write(0, 0xB0 + 1, 0xFF, 0, 0);
  write(0, 0xB0 + 2, 0xFF, 0, 0);

}
