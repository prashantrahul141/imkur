#pragma once

#include "common.hpp"
#include "imgui.h"
#include "plugins/plugin_base.hpp"
#include <filesystem>
#include <vector>

#define PLUGIN_ICON_SIZE 32

#define PLUGIN_INFO_FUNCTION_NAME "GET_PLUGIN_INFO"
typedef PluginInfo *const (*PLUGIN_INFO_FUNCTION_TYPE)();

#define PLUGIN_PUT_PIXEL_FUNCTION_NAME "PLUGIN_PUT_PIXEL"
typedef Color (*PLUGIN_PUT_PIXEL_FUNCTION_TYPE)(EditorState, ImVec2);

#define PLUGIN_REPLACE_IMAGE_FUNCTION_NAME "PLUGIN_REPLACE_IMAGE"
typedef void (*PLUGIN_REPLACE_IMAGE_FUNCTION_TYPE)(EditorState, Image,
                                                   void *data);

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
  void *replace_image_data = nullptr;
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

  /*
   * Calculate size of all vars.
   */
  static size_t calc_vars_size(Plugin &plugin);
};
