#pragma once
#include "sdhi.h"
#include "action.h"

typedef struct {
  sdhi_t sdhi;
  int32_t *values;
  actions_t actions;
  action_value_t *action_values;
} setup_t;
