#pragma once
#include "pico/stdlib.h"
#include <mapbox/variant.hpp>
#include "geometry.h"
namespace display_list {
  struct line_t {
    point_t start;
    point_t end;
    uint8_t size;
    line_t(const point_t start, const point_t end, const uint8_t size) : start(start), end(end), size(size) {}
  };

  struct circle_t {
    point_t center;
    uint8_t radius;
    circle_t(const point_t center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct filled_circle_t {
    point_t center;
    uint8_t radius;
    filled_circle_t(const point_t center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct sine_interval_t {
    point_t start;
    uint8_t length;
    uint8_t amplitude;
    /* To and from are expressed in PI / 8 */
    uint8_t from;
    uint8_t until;
    sine_interval_t(const point_t start, const uint8_t length, const uint8_t amplitude, const uint8_t from, const uint8_t until) : start(start), length(length), amplitude(amplitude), from(from), until(until) {}
  };

  typedef mapbox::util::variant<line_t, circle_t, filled_circle_t, sine_interval_t> item_t;
}
