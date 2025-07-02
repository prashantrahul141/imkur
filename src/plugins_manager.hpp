#pragma once

#include "editor.hpp"
#include "imgui.h"
#include <cstdint>
#include <filesystem>
#include <vector>

#define PLUGIN_ICON_SIZE 32

/*
 * Types a plugin can be.
 */
enum class PluginType {
  PLUGIN_TYPE_PUT_PIXEL = 0,
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
  uint8_t icon[PLUGIN_ICON_SIZE][PLUGIN_ICON_SIZE] = {{0}};
};
#define PLUGIN_INFO_FUNCTION_NAME "GET_PLUGIN_INFO"
typedef PluginInfo *const (*PLUGIN_INFO_FUNCTION_TYPE)();

#define PLUGIN_PUT_PIXEL_FUNCTION_NAME "PLUGIN_PUT_PIXEL"
typedef Color (*PLUGIN_PUT_PIXEL_FUNCTION_TYPE)(EditorState, ImVec2);

#define PLUGIN_REPLACE_IMAGE_FUNCTION_NAME "PLUGIN_REPLACE_IMAGE"
typedef void (*PLUGIN_REPLACE_IMAGE_FUNCTION_TYPE)(EditorState, Image);

/*
 * Single loaded plugin
 */
struct Plugin {
  void *handler;
  PLUGIN_INFO_FUNCTION_TYPE info_function;
  union {
    PLUGIN_PUT_PIXEL_FUNCTION_TYPE put_pixel;
    PLUGIN_REPLACE_IMAGE_FUNCTION_TYPE replace_image;
  } callback;
  Texture icon;
};

#ifdef _WIN32
#include <windows.h>
#define DL_OPEN(path) LoadLibrary(TEXT(path))
#define DL_SYMBOL(handle, sym) GetProcAddress((HINSTANCE *)handle, sym)
#define DL_LAST_ERROR() GetLastError()
#define DL_CLOSE(handle) FreeLibrary((HINSTANCE *)handle)
#else
#include <dlfcn.h>
#define DL_OPEN(path) dlopen(path, RTLD_LAZY)
#define DL_SYMBOL(handle, sym) dlsym((void *)handle, sym)
#define DL_LAST_ERROR() dlerror()
#define DL_CLOSE(handle) dlclose((void *)handle)
#endif // _WIN32

/*
 * Loads and manages plugins
 */
class PluginManager {
public:
  // all the verified loaded plugins.
  std::vector<Plugin> plugins;

public:
  /*
   * Constructor
   */
  PluginManager();

  /*
   * Destructor
   */
  ~PluginManager();

private:
  /*
   * Iterator for all plugin files.
   */
  static std::vector<std::filesystem::directory_entry> get_plugin_files();

  /*
   * verifies if the given plugin is valid or not
   */
  static bool is_plugin_valid(Plugin &plugin);

  /*
   * Loads icon for plugins
   */
  static bool load_plugin_icon(Plugin &plugin);
};
