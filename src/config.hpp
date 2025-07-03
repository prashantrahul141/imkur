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
#define UI_SIDEBAR_WIDTH 60.0f
#define UI_RIGHTBAR_WIDTH 160.0f
#define UI_BOTTOMBAR_HEIGHT 60.0f
#define UI_IMAGE_ZOOM_RATE 1.3f
#define UI_IMAGE_MAX_SCALE 10.f
#define UI_IMAGE_MIN_SCALE 0.15f
#define UI_IMAGE_SCROLL_RATE                                                   \
  10.0f / 100.0f // % of image to move when scrolled horizontally or vertically

#define UI_SWATCH_1 1.0f, 1.0f, 1.0f, 1.0f
#define UI_SWATCH_2 0.0f, 0.0f, 0.0f, 1.0f
#define UI_SWATCH_3 1.0f, 0.0f, 0.0f, 1.0f
#define UI_SWATCH_4 0.0f, 1.0f, 0.0f, 1.0f
#define UI_SWATCH_5 0.0f, 0.0f, 1.0f, 1.0f
#define UI_SWATCH_6 1.0f, 1.0f, 0.0f, 1.0f
#define UI_SWATCH_7 0.0f, 1.0f, 1.0f, 1.0f
#define UI_SWATCH_8 1.0f, 0.0f, 1.0f, 1.0f

// Paths
#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif
#define PATH_RESOURCES_DIR "../resources"
#define PATH_FONT_FILE PATH_RESOURCES_DIR PATH_SEPERATOR "Cousine-Regular.ttf"
#define PATH_PLUGINS_DIR "./plugins"

// Colors
#define COLOR_PRIMARY_BACKGROUND                                               \
  0.05882352941f, 0.05882352941f, 0.05882352941f, 1.0f
#define COLOR_SECONDARY_BACKGROUND                                             \
  0.06882352943f, 0.06882352941f, 0.06882352941f, 1.0f

#define _COLOR_ICON_R 255
#define _COLOR_ICON_G 255
#define _COLOR_ICON_B 255
#define _COLOR_ICON_A 255
#define COLOR_ICON                                                             \
  (_COLOR_ICON_R << 24) + (_COLOR_ICON_G << 16) + (_COLOR_ICON_B << 8) +       \
      _COLOR_ICON_A

// Editor
#define EDITOR_LERP_STEP_SPACING_PERCENT                                       \
  95.0 // higher value = better performance but worse results
#define EDITOR_PUT_PIXEL_DELAY_MS 50
