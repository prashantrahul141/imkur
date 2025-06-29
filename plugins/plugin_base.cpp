/*
 * This file will serve as both base and a header file for new plugins.
 * It can be imported for declarations which will be used throughout the plugin.
 */

#include <cstdint>

#define ICON_SIZE 32

#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

/*
 *  Represents one pixel.
 */
struct Color {
  uint8_t r, g, b, a;
};

/*
 * Position
 */
struct Vec2 {
  uint32_t x, y;
};

/*
 * Image
 */
struct Image {
  uint8_t *data;
  int32_t width, height, channels;
  const int32_t components_per_pixel = 4;
};

/*
 * Editor state
 */
struct EditorState {
  Color primary_selected_color;
  uint8_t opacity;
};

/*
 * Types a plugin can be.
 */
enum class PluginType {
  // This is a type of plugin which works on per pixel basis.
  // Like a pencil. You will need to define a function
  // named `PLUGIN_PUT_PIXEL` if the type is set to this.
  PLUGIN_TYPE_PUT_PIXEL = 0,

  // This is a type of plugin which works on the entire image.
  // Like a blur filter. You will need to define a function named
  // `PLUGIN_REPLACE_IMAGE` if the type if set to this.
  PLUGIN_TYPE_REPLACE_IMAGE,
};

/*
 * This struct contains the info regarding a single plugin.
 */
struct PluginInfo {
  // Name of the plugin. This will show up in the editor.
  char const *name;

  // Short description of what the plugin does. This will show up in the editor.
  char const *description;

  // Plugin type
  PluginType plugin_type;

  // 32x32 grayscale icon.
  uint8_t icon[ICON_SIZE][ICON_SIZE] = {{0}};
};

/*
 * This function should exist otherwise the plugin will be treated at invalid
 * and will not be loaded.
 * Should return the pointer to static `PluginInfo` instance.
 *
 * extern "C" EXPORT PluginInfo *const GET_PLUGIN_INFO();
 */

/*
 * if the type of the plugin is `PLUGIN_PUT_PIXEL`, this function will be called
 *  whenever the plugin is used over the image.
 *  The color returned from this will override the pixel color.
 *
 * extern "C" EXPORT Color PLUGIN_PUT_PIXEL(EditorState es, Vec2 pos);
 */

/*
 * if the type of the plugin is `PLUGIN_REPLACE_IMAGE`, this function will be
 * called to replace the image inplace.
 *
 * extern "C" EXPORT void PLUGIN_REPLACE_IMAGE(EditorState es, Image img);
 */
