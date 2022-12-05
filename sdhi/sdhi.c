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
static int32_t panel_values[9];
static int32_t panel_max[9];
static int32_t panel_min[9];

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

static void set_panel_values(const int32_t * const values, const sdhi_t sdhi) {
  for(uint8_t i = 0; i < 8; i++) {
    const sdhi_control_t * const control = find_control(sdhi.panels[current_panel].controls[i], sdhi);
    if(control != NULL) {
      panel_min[i] = control->min;
      panel_max[i] = control->max;
      panel_values[i] = values[control->id];
    }
  }
  panel_min[8] = 0;
  panel_max[8] = sdhi.panels_size - 1;
  panel_values[8] = current_panel;
}

static void set_values(int32_t * const values, const sdhi_t sdhi) {
  for(uint8_t i = 0; i < 8; i++) {
    const sdhi_control_t * const control = find_control(sdhi.panels[current_panel].controls[i], sdhi);
    if(control != NULL) {
      values[control->id] = panel_values[i];
    }
  }
}

bool sdhi_update_values_blocking(int32_t * const values, const sdhi_t sdhi) {
  set_panel_values(values, sdhi);
  bool updated = i2c_controller_update_blocking(panel_values, panel_max, panel_min);
  current_panel = panel_values[8];
  set_values(values, sdhi);
  if(updated) {
    printf("PANEL: %d\n", current_panel);
  }
  return updated;
}


static void draw_control(const sdhi_control_t * const control, const uint8_t x, const uint8_t y, const int32_t top_group, const int32_t bottom_group, const int32_t start_group, const int32_t end_group) {
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
      draw_control(control, x, y, top_group, bottom_group, start_group, end_group);
    }
  }
}
