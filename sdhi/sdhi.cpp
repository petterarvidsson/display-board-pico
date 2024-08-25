#include <stdio.h>
#include <optional>
#include <pio_display.h>
#include <sdhi.hpp>
#include <overloaded.hpp>
#define EMPTY_GROUP -2
#define NO_GROUP -1

#define WIDTH 4
#define HALF_WIDTH (WIDTH / 2)
#define ROW_TOP (32 - HALF_WIDTH)
#define ROW_BOTTOM (32 + HALF_WIDTH)
#define COLUMN_LEFT (64 - HALF_WIDTH)
#define COLUMN_RIGHT (64 + HALF_WIDTH)


namespace sdhi {
  using util::overloaded;

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

  const sdhi_control_description_t sdhi_description(const sdhi_control_t control) {
    sdhi_control_description_t *description = NULL;
    std::visit(overloaded {
        [&description] (auto control) {
          description = &control;
        }},
      control);
    return *description;
  }

  const std::optional<sdhi_control_t> find_control(const int16_t id, const sdhi_t sdhi) {
    bool found = false;
    for(auto control : sdhi.controls) {
      std::visit(overloaded {[&found, id] (auto it){
        found = it.id == id;
      }}, control);
      if(found)
        return control;
    }
    return {};
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

  static int32_t update_enumeration(const int32_t size, const int32_t value, const int32_t change) {
    return update(value, change, 0, size);
  }

  static void update_values(int32_t * const values, const int32_t * const change, i2c_controller_button_t * const button, const i2c_controller_button_t * const button_change, const sdhi_t sdhi) {

    for(uint8_t i = 0; i < sdhi.panels[current_panel].controls.size(); i++) {
      auto id = sdhi.panels[current_panel].controls[i];
      if(auto control = find_control(id, sdhi)) {
        if(change[i] != 0) {
          std::visit(overloaded {
                         [i, id, values, change] (sdhi_control_type_integer_t integer_control) {
                           values[id] = update_integer(integer_control, values[id], change[i]);
                         },
                         [i, id, values, change] (sdhi_control_type_real_t real_control) {
                           values[id] = update_real(real_control, values[id], change[i]);
                         },
                         [i, id, values, change] (sdhi_control_type_enumeration_t enumeration_control) {
                           values[id] = update_enumeration((int32_t)enumeration_control.values.size() - 1, values[id], change[i]);
                         },
                         [i, id, values, change] (sdhi_control_type_visual_enumeration_t visual_enumeration_control) {
                           values[id] = update_enumeration((int32_t)visual_enumeration_control.values.size() - 1, values[id], change[i]);
                         }
            }, *control);
        }
        if(button_change[i] != I2C_CONTROLLER_NO_CHANGE) {
          button[sdhi_description(*control).id] = button_change[i];
        }
      }
    }
    if(change[8] != 0) {
      current_panel = update(current_panel, change[8], 0, sdhi.panels.size() - 1);
    }
  }

  bool sdhi_update_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi) {
    int32_t change[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    i2c_controller_button_t button_change[] = {
      I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE,
      I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE,
      I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE, I2C_CONTROLLER_NO_CHANGE
    };
    for(uint8_t i = 0; i < sdhi.controls.size(); i++) {
      button[sdhi_description(sdhi.controls[i]).id] = I2C_CONTROLLER_NO_CHANGE;
    }
    bool updated = i2c_controller_update(change, button_change);
    update_values(values, change, button, button_change, sdhi);
    return updated;
  }

  void sdhi_init_values(int32_t * const values, i2c_controller_button_t * const button, const sdhi_t sdhi) {
    for(uint16_t i = 0; i < sdhi.controls.size(); i++) {
      const sdhi_control_t control = sdhi.controls[i];
      std::visit(overloaded {
                    [values] (sdhi_control_type_integer_t integer_control) {
                      values[integer_control.id] = integer_control.initial;
                    },
                    [values] (sdhi_control_type_real_t real_control) {
                      values[real_control.id] = 0;
                    },
                    [values] (sdhi_control_type_enumeration_t enumeration_control) {
                      values[enumeration_control.id] = enumeration_control.initial;
                    },
                    [values] (sdhi_control_type_visual_enumeration_t visual_enumeration_control) {
                      values[visual_enumeration_control.id] = visual_enumeration_control.initial;
                    }
        }, control);
      button[sdhi_description(control).id] = I2C_CONTROLLER_NO_CHANGE;
    }
  }


  int32_t sdhi_integer(const uint16_t id, const int32_t * const values, const sdhi_t sdhi) {
    return values[id];
  }

  float sdhi_real(const uint16_t id, const int32_t * const values, const sdhi_t sdhi) {
    return values[id] * std::get<sdhi_control_type_real_t>(*find_control(id, sdhi)).step;
  }

  int32_t sdhi_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi) {
    return std::get<sdhi_control_type_enumeration_t>(*find_control(id, sdhi)).values[(uint32_t)(values[id] & 0xFFFFFF)].value;
  }

  int32_t sdhi_visual_enumeration(const uint16_t id, const int32_t * const values, const sdhi_t sdhi) {
    return std::get<sdhi_control_type_visual_enumeration_t>(*find_control(id, sdhi)).values[(uint32_t)(values[id] & 0xFFFFFF)].value;
  }

  static void display_list_item(const uint8_t x_offset, const uint8_t y_offset, const display_list::Item item, const uint8_t display) {
    uint8_t * const fb = pio_display_get(display);
    std::visit(overloaded {
        [x_offset, y_offset, fb] (display_list::Line line) {
          pio_display_draw_line(fb, line.start.x + x_offset, line.start.y + y_offset, line.end.x  + x_offset, line.end.y  + y_offset, line.size);
        },
        [x_offset, y_offset, fb] (display_list::Circle circle) {
          pio_display_draw_circle(fb, circle.center.x + x_offset, circle.center.y + y_offset, circle.radius);
        },
        [x_offset, y_offset, fb] (display_list::FilledCircle circle) {
          pio_display_draw_filled_circle(fb, circle.center.x + x_offset, circle.center.y + y_offset, circle.radius);
        },
        [x_offset, y_offset, fb] (display_list::SineSegment sine) {
          pio_display_draw_sine(fb, sine.from, sine.until, sine.start.x + x_offset, sine.start.y + y_offset, sine.length, sine.amplitude);
        }
      }, item);
  }

  static void pio_display_list(const uint8_t x_offset, const uint8_t y_offset, const tcb::span<display_list::Item> list, const uint8_t display) {
    for(auto item : list) {
      display_list_item(x_offset, y_offset, item, display);
    }
  };


  static void draw_control(const std::optional<sdhi_control_t> control_opt, const uint8_t x, const uint8_t y, const int32_t top_group, const int32_t bottom_group, const int32_t start_group, const int32_t end_group, const int32_t * const values) {
    int32_t group = EMPTY_GROUP;
    uint8_t top_start = x * 2 + y * 11;
    uint8_t top = x * 2 + 1 + y * 11;
    uint8_t top_end = x * 2 + 2 + y * 11;
    uint8_t start = x + y * 11 + 7;
    uint8_t end = x + 1 + y * 11 + 7;
    uint8_t bottom_start = x * 2 + (y + 1) * 11;
    uint8_t bottom = x * 2 + 1 + (y + 1) * 11;
    uint8_t bottom_end = x * 2 + 2 + (y + 1) * 11;

    if(auto control = control_opt) {
      group = sdhi_description(*control).group;
      pio_display_print_center(pio_display_get(top), 0, SIZE_13, true, sdhi_description(*control).title);

      std::visit(overloaded {
          [values, bottom] (sdhi_control_type_integer_t integer_control) {
            char value[16];
            snprintf(value, 16, "%d", values[integer_control.id]);
            pio_display_print_center(pio_display_get(bottom), 63 - 13 - 8, SIZE_13, true, value);
            int32_t min = integer_control.min;
            int32_t max = integer_control.max;

            uint32_t total = (uint8_t)((float)(values[integer_control.id] - min) / (float)(max - min) * 96);
            uint32_t middle = (uint8_t)((float)(integer_control.middle - min) / (float)(max - min) * 96);
            uint32_t start;
            uint32_t end;
            if(values[integer_control.id] <= integer_control.middle) {
              start = 17 + total;
              end = 17 + middle;
            } else {
              start = 17 + middle;
              end = 17 + total;
            }
            pio_display_fill_rectangle(pio_display_get(bottom), start, 63 - 4, end, 63);
          },
          [values, bottom] (sdhi_control_type_real_t real_control) {
            char value[16];
            snprintf(value, 16, "%.2f", values[real_control.id] * real_control.step);
            pio_display_print_center(pio_display_get(bottom), 63 - 13 - 8, SIZE_13, true, value);
            int32_t min = (int32_t)(real_control.min / real_control.step);
            int32_t max = (int32_t)(real_control.max / real_control.step);
            pio_display_fill_rectangle(pio_display_get(bottom), 16, 63 - 4, 16 + (uint8_t)((float)(values[real_control.id] - min) / (float)(max - min) * 96), 63);
          },
          [values, bottom] (sdhi_control_type_enumeration_t enumeration_control) {
            pio_display_print_center(pio_display_get(bottom), 63 - 13, SIZE_13, true, enumeration_control.values[(uint32_t)(values[enumeration_control.id] & 0xFFFFFF)].name);
          },
          [values, bottom] (sdhi_control_type_visual_enumeration_t enumeration) {
            const EnumVisual value = enumeration.values[(uint32_t)(values[enumeration.id] & 0xFFFFFF)];
            pio_display_list((127 - enumeration.width) / 2, 63 - 28, value.display_list, bottom);
          }
        }, *control);
    }

    if(group == NO_GROUP || group != top_group) {
      draw_right_row(pio_display_get(top_start));
      draw_row(pio_display_get(top));
      draw_left_row(pio_display_get(top_end));
    }

    if(group == NO_GROUP || group != bottom_group) {
      draw_right_row(pio_display_get(bottom_start));
      draw_row(pio_display_get(bottom));
      draw_left_row(pio_display_get(bottom_end));
    }

    if(group == NO_GROUP || group != start_group) {
      draw_lower_column(pio_display_get(top_start));
      draw_row(pio_display_get(start));
      draw_upper_column(pio_display_get(bottom_start));
    }

    if(group == NO_GROUP || group != end_group) {
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
    pio_display_print_center(pio_display_get(bottom), 63 - 26, SIZE_13, true, sdhi.panels[current_panel].subtitle);

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
      return EMPTY_GROUP;
    } else {
      int32_t control_id = sdhi.panels[current_panel].controls[control_index(x, y)];
      if(control_id == -1) {
        return EMPTY_GROUP;
      } else {
        return sdhi_description(*find_control(control_id, sdhi)).group;
      }
    }
  }

  void sdhi_update_displays(const int32_t * const values, const sdhi_t sdhi) {
    pio_display_clear_current_framebuffer();
    const sdhi_panel_t panel = sdhi.panels[current_panel];
    for(uint8_t x = 0; x < 3; x++) {
      for(uint8_t y = 0; control_index(x, y) < 8; y++) {
        uint8_t i = control_index(x, y);
        const int32_t top_group = find_group(x, y - 1, sdhi);
        const int32_t bottom_group = find_group(x, y + 1, sdhi);
        const int32_t start_group = find_group(x - 1, y, sdhi);
        const int32_t end_group = find_group(x + 1, y, sdhi);
        draw_control(find_control(panel.controls[i], sdhi), x, y, top_group, bottom_group, start_group, end_group, values);
      }
    }
    if(panel.display_list_generator != NULL) {
      const tcb::span<display_list::Item> list = (*panel.display_list_generator)(values, (void*)&sdhi);
      pio_display_list(0, 0, list, panel.generated_display);
    }

    draw_panel_control(sdhi);
  }

}
