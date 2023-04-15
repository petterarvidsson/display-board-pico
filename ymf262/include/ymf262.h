#pragma once

#include <stdint.h>

typedef enum {
  FM,
  AM,
  FMFM,
  FM_FM,
  AM_FM,
  AM_FM_AM
} ymf262_channel_connection_t;

typedef enum {
  CONN_SEL,
  AM_1,
  AM_2,
  AM_3,
  AM_4,
  VIB_1,
  VIB_2,
  VIB_3,
  VIB_4,
  EGT_1,
  EGT_2,
  EGT_3,
  EGT_4,
  KSR_1,
  KSR_2,
  KSR_3,
  KSR_4,
  MULT_1,
  MULT_2,
  MULT_3,
  MULT_4,
  KSL_1,
  KSL_2,
  KSL_3,
  KSL_4,
  TL_1,
  TL_2,
  TL_3,
  TL_4,
  AR_1,
  AR_2,
  AR_3,
  AR_4,
  DR_1,
  DR_2,
  DR_3,
  DR_4,
  SL_1,
  SL_2,
  SL_3,
  SL_4,
  RR_1,
  RR_2,
  RR_3,
  RR_4,
  FNUM_L,
  KON,
  BLOCK,
  FNUM_H,
  CHD,
  CHC,
  CHB,
  CHA,
  FB,
  CNT_1,
  CNT_3,
  WS_1,
  WS_2,
  WS_3,
  WS_4
} ymf262_parameter_t;

typedef struct {
  ymf262_parameter_t parameter;
  uint8_t value;
} ymf262_parameter_value_t;


void ymf262_parameter(uint8_t c, ymf262_parameter_t parameter, uint8_t value);
void ymf262_all_channels_parameter(ymf262_parameter_t parameter, uint8_t value);
void ymf262_parameters(uint8_t c, const ymf262_parameter_value_t * const parameter_values, const uint32_t size);
void ymf262_channel_connection(uint8_t c, ymf262_channel_connection_t type);
void ymf262_start(uint8_t c);
void ymf262_frequency(uint8_t c, uint8_t frequency);
void ymf262_stop(uint8_t c);
void ymf262_init(void);
