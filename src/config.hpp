#pragma once

// NHLOG_OFF
#define APPLICATION_DEBUG_LEVEL NHLOG_DEBUG

// UI
#define UI_TARGET_FPS 10
#define UI_WINDOW_TITLE "imkur"
#define UI_INIT_WINDOW_WIDTH 1024
#define UI_INIT_WINDOW_HEIGHT 768
#define UI_ENABLE_VSYNC true
#define UI_ENABLE_ANTIALIASING true
#define UI_ANTIALIASING_MUT 8
#define UI_SIDEBAR_WIDTH 30.0f
#define UI_IMAGE_ZOOM_RATE 1.3f
#define UI_IMAGE_MAX_SCALE 10.f
#define UI_IMAGE_MIN_SCALE 0.15f
#define UI_IMAGE_SCROLL_RATE                                                   \
  10.0f / 100.0f // % of image to move when scrolled horizontally or vertically

// Paths
#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif
#define PATH_RESOURCES_DIR "../resources"
#define PATH_FONT_FILE PATH_RESOURCES_DIR PATH_SEPERATOR "Cousine-Regular.ttf"

// Colors
#define COLOR_PRIMARY_BACKGROUND                                               \
  0.05882352941f, 0.05882352941f, 0.05882352941f, 1.00f
