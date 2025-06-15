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

// Paths
#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif
#define PATH_RESOURCES_DIR "../resources"
#define PATH_FONT_FILE PATH_RESOURCES_DIR PATH_SEPERATOR "Cousine-Regular.ttf"

// Colors
#define COLOR_PRIMARY_BACKGROUND 0.45f, 0.55f, 0.60f, 1.00f
