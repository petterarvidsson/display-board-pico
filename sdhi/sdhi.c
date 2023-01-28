#include <stdio.h>
#include <i2c_controller.h>
#include <pio_display.h>
#include <sdhi.h>

#define WIDTH 4
#define HALF_WIDTH (WIDTH / 2)
#define ROW_TOP (32 - HALF_WIDTH)
#define ROW_BOTTOM (32 + HALF_WIDTH)
#define COLUMN_LEFT (64 - HALF_WIDTH)
#define COLUMN_RIGHT (64 + HALF_WIDTH)

static void draw_lower_column(uint8_t * const fd) {
    pio_display_fill_rectangle(fd, COLUMN_LEFT, 0, COLUMN_RIGHT, ROW_TOP);
}

static void draw_upper_column(uint8_t *const fd) {
    pio_display_fill_rectangle(fd, COLUMN_LEFT, ROW_TOP, COLUMN_RIGHT, 63);
}

static void draw_right_row(uint8_t * const fd) {
    pio_display_fill_rectangle(fd, COLUMN_LEFT, ROW_TOP, 127, ROW_BOTTOM);
}

static void draw_left_row(uint8_t * const fd) {
    pio_display_fill_rectangle(fd, 0, ROW_TOP, COLUMN_RIGHT, ROW_BOTTOM);
}

static void draw_row(uint8_t * const fd) {
    pio_display_fill_rectangle(fd, 0, ROW_TOP, 127, ROW_BOTTOM);
}

static void draw_column(uint8_t * const fd) {
    pio_display_fill_rectangle(fd, COLUMN_LEFT, 0, COLUMN_RIGHT, 63);
}

static uint32_t current_panel;

void sdhi_init(const sdhi_t sdhi) {
  current_panel = 0;
}

static const sdhi_control_t * const find_control(const int16_t id, const sdhi_t sdhi) {
  for(uint8_t i = 0; i < sdhi.controls_size; i++) {
    if(sdhi.controls[i].id == id) {
      return &(sdhi.controls[i]);
    }
  }
  return NULL;
}

static int32_t update(const int32_t value, const int32_t change, const int32_t min, const int32_t max) {
  int32_t update = value + change;
  if(update < min) {
    update = min;
  }
  if(update > max) {
    update = max;
  }
  return update;
}

static int32_t update_integer(const sdhi_control_type_integer_t integer, const int32_t value, const int32_t change) {
  return update(value, change, integer.min, integer.max);
}

static int32_t update_real(const sdhi_control_type_real_t real, const int32_t value, const int32_t change) {
  return update(value, change, (int32_t)(real.min / real.step), (int32_t)(real.max / real.step));
}

static int32_t update_enumeration(const sdhi_control_type_enumeration_t enumeration, const int32_t value, const int32_t change) {
  const int32_t max = (int32_t)enumeration.size - 1;
  int8_t acc = (value >> 24) & 0xFF;
  acc += change;
  int32_t i = value & 0xFFFFFF;
  if(i == 0 && acc < 0) {
    acc = 0;
  } else if(i == max && acc > 0) {
    acc = 0;
  }

  if(acc >= 3) {
    i = update(i, 1, 0, max);
    acc = 0;
  } else if(acc <= -3){
    i = update(i, -1, 0, max);
    acc = 0;
  }
  return (acc << 24) + i;
}

static void update_values(int32_t * const values, const int32_t * const change, const sdhi_t sdhi) {
  for(uint8_t i = 0; i < 8; i++) {
    const sdhi_control_t * const control = find_control(sdhi.panels[current_panel].controls[i], sdhi);
    if(control != NULL) {
      if(change[i] != 0) {
        switch(control->type) {
        case SDHI_CONTROL_TYPE_INTEGER:
          values[control->id] = update_integer(control->configuration.integer, values[control->id], change[i]);
          break;
        case SDHI_CONTROL_TYPE_REAL:
          values[control->id] = update_real(control->configuration.real, values[control->id], change[i]);
        break;
        case SDHI_CONTROL_TYPE_ENUMERATION:
          values[control->id] = update_enumeration(control->configuration.enumeration, values[control->id], change[i]);
          break;
        }
      }
    }
  }
  if(change[8] != 0) {
    current_panel = update(current_panel, change[8], 0, sdhi.panels_size - 1);
  }
}

bool sdhi_update_values(int32_t * const values, const sdhi_t sdhi) {
  int32_t change[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  bool updated = i2c_controller_update(change);
  update_values(values, change, sdhi);
  return updated;
}

void sdhi_init_values(int32_t * const values, const sdhi_t sdhi) {
  for(uint16_t i = 0; i < sdhi.controls_size; i++) {
    const sdhi_control_t control = sdhi.controls[i];
    switch(control.type) {
    case SDHI_CONTROL_TYPE_INTEGER:
      values[control.id] = control.configuration.integer.initial;
      break;
    case SDHI_CONTROL_TYPE_REAL:
      values[control.id] = 0;
      break;
    case SDHI_CONTROL_TYPE_ENUMERATION:
      values[control.id] = control.configuration.enumeration.initial;
      break;
    }
  }
}

int32_t sdhi_integer(const uint16_t id, int32_t * const values, const sdhi_t sdhi) {
  return values[id];
}
float sdhi_real(const uint16_t id, int32_t * const values, const sdhi_t sdhi) {
  return values[id] * find_control(id, sdhi)->configuration.real.step;
}

int32_t sdhi_enumeration(const uint16_t id, int32_t * const values, const sdhi_t sdhi) {
  return find_control(id, sdhi)->configuration.enumeration.values[(uint32_t)(values[id] & 0xFFFFFF)].value;
}

static void draw_control(const sdhi_control_t * const control, const uint8_t x, const uint8_t y, const int32_t top_group, const int32_t bottom_group, const int32_t start_group, const int32_t end_group, const int32_t * const values) {
  int32_t group = -1;
  uint8_t top_start = x * 2 + y * 11;
  uint8_t top = x * 2 + 1 + y * 11;
  uint8_t top_end = x * 2 + 2 + y * 11;
  uint8_t start = x + y * 11 + 7;
  uint8_t end = x + 1 + y * 11 + 7;
  uint8_t bottom_start = x * 2 + (y + 1) * 11;
  uint8_t bottom = x * 2 + 1 + (y + 1) * 11;
  uint8_t bottom_end = x * 2 + 2 + (y + 1) * 11;

  if(control != NULL) {
    group = control->group;
    pio_display_print_center(pio_display_get(top), 0, SIZE_13, true, control->title);
    switch(control->type) {
    case SDHI_CONTROL_TYPE_INTEGER: {
      char value[16];
      snprintf(value, 16, "%d", values[control->id]);
      pio_display_print_center(pio_display_get(bottom), 63 - 13 - 8, SIZE_13, true, value);
      int32_t min = control->configuration.integer.min;
      int32_t max = control->configuration.integer.max;
      pio_display_fill_rectangle(pio_display_get(bottom), 16, 63 - 4, 16 + (uint8_t)((float)(values[control->id] - min) / (float)(max - min) * 96), 63);
      break;
    }
    case SDHI_CONTROL_TYPE_REAL: {
      char value[16];
      snprintf(value, 16, "%.2f", values[control->id] * control->configuration.real.step);
      pio_display_print_center(pio_display_get(bottom), 63 - 13 - 8, SIZE_13, true, value);
      int32_t min = (int32_t)(control->configuration.real.min / control->configuration.real.step);
      int32_t max = (int32_t)(control->configuration.real.max / control->configuration.real.step);
      pio_display_fill_rectangle(pio_display_get(bottom), 16, 63 - 4, 16 + (uint8_t)((float)(values[control->id] - min) / (float)(max - min) * 96), 63);
      break;
    }
    case SDHI_CONTROL_TYPE_ENUMERATION:
      pio_display_print_center(pio_display_get(bottom), 63 - 13, SIZE_13, true, control->configuration.enumeration.values[(uint32_t)(values[control->id] & 0xFFFFFF)].name);
      break;
    }
  }

  if(group != top_group) {
    draw_right_row(pio_display_get(top_start));
    draw_row(pio_display_get(top));
    draw_left_row(pio_display_get(top_end));
  }

  if(group != bottom_group) {
    draw_right_row(pio_display_get(bottom_start));
    draw_row(pio_display_get(bottom));
    draw_left_row(pio_display_get(bottom_end));
  }

  if(group != start_group) {
    draw_lower_column(pio_display_get(top_start));
    draw_row(pio_display_get(start));
    draw_upper_column(pio_display_get(bottom_start));
  }

  if(group != end_group) {
    draw_lower_column(pio_display_get(top_end));
    draw_row(pio_display_get(end));
    draw_upper_column(pio_display_get(bottom_end));
  }

}

static void draw_panel_control(const sdhi_t sdhi) {
  uint8_t top_start = 2 * 2 + 2 * 11;
  uint8_t top = 2 * 2 + 1 + 2 * 11;
  uint8_t top_end = 2 * 2 + 2 + 2 * 11;
  uint8_t start = 2 + 2 * 11 + 7;
  uint8_t end = 2 + 1 + 2 * 11 + 7;
  uint8_t bottom_start = 2 * 2 + (2 + 1) * 11;
  uint8_t bottom = 2 * 2 + 1 + (2 + 1) * 11;
  uint8_t bottom_end = 2 * 2 + 2 + (2 + 1) * 11;

  pio_display_print_center(pio_display_get(top), 0, SIZE_13, true, sdhi.panel_selector_title);
  pio_display_print_center(pio_display_get(bottom), 63 - 13, SIZE_13, true, sdhi.panels[current_panel].title);

  draw_right_row(pio_display_get(top_start));
  draw_row(pio_display_get(top));
  draw_left_row(pio_display_get(top_end));

  draw_right_row(pio_display_get(bottom_start));
  draw_row(pio_display_get(bottom));
  draw_left_row(pio_display_get(bottom_end));

  draw_lower_column(pio_display_get(top_start));
  draw_row(pio_display_get(start));
  draw_upper_column(pio_display_get(bottom_start));

  draw_lower_column(pio_display_get(top_end));
  draw_row(pio_display_get(end));
  draw_upper_column(pio_display_get(bottom_end));

}

static uint8_t control_index(uint8_t x, uint8_t y) {
  return y * 3 + x;
}

static int32_t find_group(int8_t x, int8_t y, const sdhi_t sdhi) {
  if(x < 0 || y < 0 || x > 2 || y > 2 || (x == 2 && y == 2)) {
    return -1;
  } else {
    int32_t control_id = sdhi.panels[current_panel].controls[control_index(x, y)];
    if(control_id == -1) {
      return control_id;
    } else {
      return find_control(control_id, sdhi)->group;
    }
  }
}

void sdhi_update_displays(const int32_t * const values, const sdhi_t sdhi) {
  pio_display_clear_current_framebuffer();
  const sdhi_panel_t panel = sdhi.panels[current_panel];
  for(uint8_t x = 0; x < 3; x++) {
    for(uint8_t y = 0; control_index(x, y) < 8; y++) {
      uint8_t i = control_index(x, y);
      const sdhi_control_t * const control = find_control(sdhi.panels[current_panel].controls[i], sdhi);
      const int32_t top_group = find_group(x, y - 1, sdhi);
      const int32_t bottom_group = find_group(x, y + 1, sdhi);
      const int32_t start_group = find_group(x - 1, y, sdhi);
      const int32_t end_group = find_group(x + 1, y, sdhi);
      draw_control(control, x, y, top_group, bottom_group, start_group, end_group, values);
    }
  }
  draw_panel_control(sdhi);
}
