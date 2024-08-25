#pragma once
#include "pico/stdlib.h"
#include <variant>

namespace display_list {
  struct Point {
    uint8_t x;
    uint8_t y;
    Point(const uint8_t x, const uint8_t y) : x(x), y(y) {}
  };
  struct Line {
    Point start;
    Point end;
    uint8_t size;
    Line(const Point start, const Point end, const uint8_t size) : start(start), end(end), size(size) {}
  };

  struct Circle {
    Point center;
    uint8_t radius;
    Circle(const Point center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct FilledCircle {
    Point center;
    uint8_t radius;
    FilledCircle(const Point center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct SineSegment {
    Point start;
    uint8_t length;
    uint8_t amplitude;
    /* To and from are expressed in PI / 8 */
    uint8_t from;
    uint8_t until;
    SineSegment(const Point start, const uint8_t length, const uint8_t amplitude, const uint8_t from, const uint8_t until) : start(start), length(length), amplitude(amplitude), from(from), until(until) {}
  };

  using Item = std::variant<Line, Circle, FilledCircle, SineSegment>;
}
