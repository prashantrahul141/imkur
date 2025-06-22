#include "common.hpp"
#include "glad/glad.h"
#include "imgui.h"
#include <cstdint>
#include <vector>

ImVec4 ColorToImVec4(Color c) {
  return ImVec4(
      static_cast<float>(c.r / 255.0f), static_cast<float>(c.g / 255.0f),
      static_cast<float>(c.b / 255.0f), static_cast<float>(c.a / 255.0f));
}

Color ImVec4ToColor(ImVec4 c) {
  return Color{.r = static_cast<uint8_t>(c.x * 255.0f),
               .g = static_cast<uint8_t>(c.y * 255.0f),
               .b = static_cast<uint8_t>(c.z * 255.0f),
               .a = static_cast<uint8_t>(c.w * 255.0)};
}

[[nodiscard]] std::vector<Vec2<std::int32_t>>
get_surrounding_pixels(Vec2<std::int32_t> &center_pos,
                       int32_t surround_circle_radius, Image &img) {

  std::int32_t radius_sqr = surround_circle_radius * surround_circle_radius;
  std::vector<Vec2<std::int32_t>> vec;
  auto min_x = std::max(center_pos.x - surround_circle_radius, 0);
  auto max_x = std::min(center_pos.x + surround_circle_radius, img.width - 1);

  auto min_y = std::max(center_pos.y - surround_circle_radius, 0);
  auto max_y = std::max(center_pos.y + surround_circle_radius, img.width - 1);

  for (std::int32_t y = min_y; y <= max_y; ++y) {
    for (std::int32_t x = min_x; x <= max_x; ++x) {
      std::int32_t dx = x - center_pos.x;
      std::int32_t dy = y - center_pos.y;
      if (dx * dx + dy * dy <= radius_sqr) {
        vec.push_back(Vec2(x, y));
      }
    }
  }

  return vec;
}
