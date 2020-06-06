#pragma once

#include <cstdint>

namespace graphics
{
struct Color_t
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

static constexpr Color_t kWhite =
    Color_t{ .red = 0xFF, .green = 0xFF, .blue = 0xFF };
static constexpr Color_t kBlack =
    Color_t{ .red = 0x00, .green = 0x00, .blue = 0x00 };
static constexpr Color_t kRed =
    Color_t{ .red = 0xFF, .green = 0x00, .blue = 0x00 };
static constexpr Color_t kGreen =
    Color_t{ .red = 0x00, .green = 0xFF, .blue = 0x00 };
static constexpr Color_t kBlue =
    Color_t{ .red = 0x00, .green = 0x00, .blue = 0xFF };

struct Point_t
{
  uint16_t x;
  uint16_t y;
};

struct Size_t
{
  size_t width;
  size_t height;
};

struct Frame_t
{
  Point_t origin;
  Size_t size;

  explicit Frame_t(uint16_t x, uint16_t y, size_t width, size_t height)
      : origin({ .x = x, .y = y }), size({ .width = width, .height = height })
  {
  }
};
}  // namespace graphics
