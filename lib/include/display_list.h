#pragma once
#include "pico/stdlib.h"
#include "geometry.h"

typedef enum {
  DISPLAY_LIST_LINE = 0,
  DISPLAY_LIST_CIRCLE = 1,
  DISPLAY_LIST_FILLED_CIRCLE = 2
} display_list_item_type_t;

typedef struct {
  point_t start;
  point_t end;
  uint8_t size;
} display_list_item_line_t;

typedef struct {
  point_t center;
  uint8_t radius;
} display_list_item_circle_t;

typedef union {
  display_list_item_line_t line;
  display_list_item_circle_t circle;
} display_list_item_configuration_t;

typedef struct {
  uint8_t display;
  display_list_item_type_t type;
  display_list_item_configuration_t configuration;
} display_list_item_t;

typedef struct {
  display_list_item_t *items;
  uint8_t size;
} display_list_t;
