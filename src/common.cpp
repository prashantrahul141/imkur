#include "common.hpp"
#include "glad/glad.h"
#include "imgui.h"
#include <cstdint>

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
