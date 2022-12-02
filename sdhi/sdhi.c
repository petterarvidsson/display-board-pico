#include <stdio.h>
#include <i2c_controller.h>
#include <sdhi.h>

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
