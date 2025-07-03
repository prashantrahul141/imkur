#pragma once

#include "glad/glad.h"
#include "imgui.h"
#include "plugin_base.hpp"
#include <cmath>
#include <vector>

#ifndef BREAKPOINT
#if defined(_MSC_VER)
#define BREAKPOINT() __debugbreak()
#elif defined(__clang__)
#define BREAKPOINT() __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define BREAKPOINT() __asm__ volatile("int3;nop")
#elif defined(__GNUC__) && defined(__thumb__)
#define BREAKPOINT() __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define BREAKPOINT() __asm__ volatile(".inst 0xe7f001f0")
#else
#if defined(__cplusplus)
#include <cassert>
#else
#include <assert.h>
#endif
#define BREAKPOINT() assert(0)
#endif
#endif // #ifndef BREAKPOINT

struct Texture {
  GLuint texture_id;
};

ImVec4 ColorToImVec4(Color c);
Color ImVec4ToColor(ImVec4 c);

template <typename T> class Vec2 {
public:
  T x, y;
  Vec2() : x(0), y(0) {}
  Vec2(ImVec2 vec) : x((T)vec.x), y((T)vec.y) {}
  Vec2(T x, T y) : x(x), y(y) {}
  Vec2<T> operator+(const Vec2<T> &other) {
    return Vec2(this->x + other.x, this->y + other.y);
  }
  Vec2<T> operator-(const Vec2<T> &other) {
    return Vec2(this->x - other.x, this->y - other.y);
  }
  Vec2<T> operator*(const Vec2<T> &other) {
    return Vec2(this->x * other.x, this->y * other.y);
  }
  Vec2<T> operator/(const Vec2<T> &other) {
    return Vec2(this->x / other.x, this->y / other.y);
  }
  const float dist() {
    return std::sqrt(this->x * this->x + this->y * this->y);
  }
  Vec2<T> operator*(T val) {
    return Vec2<T>((T)this->x * val, (T)this->y * val);
  }

  static Vec2<T> lerp(Vec2<T> &a, Vec2<T> &b, float t) {
    return {a.x + static_cast<T>((b.x - a.x) * t),
            a.y + static_cast<T>((b.y - a.y) * t)};
  }
  ImVec2 to_imvec2() { return ImVec2((float)this->x, (float)this->y); }
};

[[nodiscard]] std::vector<Vec2<std::int32_t>>
get_surrounding_pixels(Vec2<std::int32_t> &center_pos,
                       int32_t surround_circle_radius, Image &img);
