#pragma once

#include "glad/glad.h"
#include "imgui.h"
#include <cassert>
#include <cstdint>

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
#define BREAKPOINT() assert(0)
#endif
#endif // #ifndef BREAKPOINT

struct Texture {
  GLuint texture_id;
};

struct Color {
  uint8_t r, g, b, a;
};

ImVec4 ColorToImVec4(Color c);
Color ImVec4ToColor(ImVec4 c);
