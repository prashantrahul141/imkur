#pragma once

#include "imgui.h"
#include <cstdint>

struct Color {
  uint8_t r, g, b, a;
};

ImVec4 ColorToImVec4(Color c);
Color ImVec4ToColor(ImVec4 c);
