#include "pico/stdlib.h"
#include "stdio.h"
#include "pio_display.h"
#include "i2c_controller.h"


static int32_t values[9];
static const int32_t max[9] = {127, 127, 127, 127, 127, 127, 127, 127, 127};
static const int32_t min[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int main() {
    stdio_init_all();
    pio_display_init();
    i2c_controller_init();
    absolute_time_t start;
    absolute_time_t end;
    uint32_t us = 0;
    start = get_absolute_time();
    uint8_t x = 0, y = 0;
    int8_t dx = 1, dy = 1;
    pio_display_update();
    for(uint32_t i = 0;;) {
      if(pio_display_can_wait_without_blocking()) {
        pio_display_wait_for_finish_blocking();
        end = get_absolute_time();
        i++;
        us += absolute_time_diff_us(start, end);
        if(i % 1 == 0) {
          for(uint8_t i = 0; i < 40; i++) {
            uint8_t *display = pio_display_get(i);
            pio_display_clear(display);
            //pio_display_fill_rectangle(display, x, y, x + 4, y + 4);
            pio_display_printc(display, x, y, SIZE_13, true, 'P');
            //pio_display_fill_rectangle(display, 0, 0, 4, 4);

          }
          x = x + dx;
          y = y + dy;
          if(x > 108) {
          dx = -1;
          }
          if(x < 1) {
            dx = 1;
          }
          if(y > 44) {
            dy = -1;
          }
          if(y < 1) {
            dy = 1;
          }
        }                                       \
        start = get_absolute_time();
        pio_display_update();
      }
      uint8_t changed = i2c_controller_update_blocking(values, max, min);
      if(changed) {
        for(uint8_t i = 0; i < 9; i++) {
          printf("E%d: %d ",i ,values[i]);
        }
        printf("\n%d\n", us / i);
      }
    }
}
