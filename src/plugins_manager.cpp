#include "src/plugins_manager.hpp"
#include "nhlog.h"
#include "plugin_base.hpp"
#include "src/config.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>

/*
 * Constructor
 */
PluginManager::PluginManager() {
  for (const auto &plugin_path : get_plugin_files()) {
    nhlog_debug("PluginManager: loading = %s", plugin_path.path().c_str());

    if (!plugin_path.is_regular_file()) {
      nhlog_error("PluginManager: %s is not a regular file",
                  plugin_path.path().c_str());
      continue;
    }

    std::filesystem::path abs_path =
        std::filesystem::absolute(plugin_path.path());

    Plugin p = Plugin();
    p.handler = DL_OPEN(abs_path.c_str());
    if (nullptr == p.handler) {
      nhlog_error("PluginManager: plugin handler is null for %s",
                  abs_path.c_str());
      nhlog_error("PluginManager: plugin loading failed due to = %s",
                  DL_LAST_ERROR());
      continue;
    }

    if (!PluginManager::is_plugin_valid(p)) {
      nhlog_error("PluginManager: invalid plugin = %s",
                  plugin_path.path().c_str());
      DL_CLOSE(abs_path.c_str());
      continue;
    }

    if (!PluginManager::load_plugin_icon(p)) {
      continue;
    }

    this->plugins.push_back(p);
  }
}
/*
 *  Destructor
 */
PluginManager::~PluginManager() {
  nhlog_debug("PluginManager: destroying plugin manager");
  for (auto plugin : this->plugins) {
    nhlog_debug("PluginManager: unloading plugin = %s",
                plugin.info_function()->name);
    glDeleteTextures(1, &plugin.icon.texture_id);
    if (PLUGIN_TYPE_REPLACE_IMAGE == plugin.info_function()->plugin_type &&
        nullptr != plugin.replace_image_data) {
      free(plugin.replace_image_data);
    }
    DL_CLOSE(plugin.handler);
  }
}

/*
 * verifies if the given plugin is valid or not
 */
bool PluginManager::is_plugin_valid(Plugin &plugin) {
  plugin.info_function = (PLUGIN_INFO_FUNCTION_TYPE)DL_SYMBOL(
      plugin.handler, PLUGIN_INFO_FUNCTION_NAME);

  if (nullptr == plugin.info_function) {
    nhlog_error("PluginManager: info_function is null");
    return false;
  }
  PluginInfo *info = plugin.info_function();
  if (nullptr == info) {
    nhlog_error("PluginManager: Invalid info function");
    return false;
  }
  nhlog_debug("PluginManager: plugin name = %s", info->name);

  plugin.replace_image_data = nullptr;
  switch (info->plugin_type) {
  case PluginType::PLUGIN_TYPE_PUT_PIXEL: {
    plugin.callback.put_pixel = (PLUGIN_PUT_PIXEL_FUNCTION_TYPE)DL_SYMBOL(
        plugin.handler, PLUGIN_PUT_PIXEL_FUNCTION_NAME);

    if (nullptr == plugin.callback.put_pixel) {
      nhlog_error("PluginManager: put_pixel function was null.");
      return false;
    }
    break;
  }
  case PluginType::PLUGIN_TYPE_REPLACE_IMAGE: {
    plugin.callback.replace_image =
        (PLUGIN_REPLACE_IMAGE_FUNCTION_TYPE)DL_SYMBOL(
            plugin.handler, PLUGIN_REPLACE_IMAGE_FUNCTION_NAME);

    if (nullptr == plugin.callback.replace_image) {
      nhlog_error("PluginManager: replace_image function was null.");
      return false;
    }
    // allocate memmory for meta vars
    plugin.replace_image_data = malloc(PluginManager::calc_vars_size(plugin));
    break;
  }
  default: {
    nhlog_error("PluginManager: invalid plugin type");
    return false;
  }
  }

  return true;
}

/*
 * Loads icon for plugins
 */
bool PluginManager::load_plugin_icon(Plugin &p) {
  GLubyte data[PLUGIN_ICON_SIZE * PLUGIN_ICON_SIZE * 4];
  for (int y = 0; y < PLUGIN_ICON_SIZE; ++y) {
    for (int x = 0; x < PLUGIN_ICON_SIZE; ++x) {
      int idx = (y * PLUGIN_ICON_SIZE + x) * 4;
      auto color = p.info_function()->icon[y][x] ? COLOR_ICON : 0;
      std::memcpy(&data[idx], &color, 4);
    }
  }

  glGenTextures(1, &p.icon.texture_id);
  glBindTexture(GL_TEXTURE_2D, p.icon.texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PLUGIN_ICON_SIZE, PLUGIN_ICON_SIZE, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, data);

  GLenum err = glGetError();
  if (err != 0) {
    nhlog_fatal("PluginManager: Icon failed to upload to gpu");
    return false;
  }
  return true;
}

/*
 * Iterator for all plugin files.
 */
std::vector<std::filesystem::directory_entry>
PluginManager::get_plugin_files() {
  std::vector<std::filesystem::directory_entry> files;
  for (const auto &plugin_path : std::filesystem::directory_iterator(".")) {
    files.push_back(plugin_path);
  }

  if (std::filesystem::exists(PATH_PLUGINS_DIR) &&
      std::filesystem::is_directory(PATH_PLUGINS_DIR)) {
    for (const auto &plugin_path :
         std::filesystem::directory_iterator(PATH_PLUGINS_DIR)) {
      files.push_back(plugin_path);
    }
  } else {
    nhlog_error("PluginManager: No plugins directory found.");
  }

  files.erase(std::remove_if(files.begin(), files.end(),
                             [](std::filesystem::directory_entry entry) {
                               return !(entry.exists() &&
                                        entry.is_regular_file() &&
                                        entry.path().has_extension() &&
                                        (entry.path().extension() == ".so" ||
                                         entry.path().extension() == ".dylib" ||
                                         entry.path().extension() == ".dll"));
                             }),
              files.end());

  nhlog_debug("Plugin files: ");
  for (const auto &file : files) {
    nhlog_debug("\t%s", file.path().c_str());
  }
  return files;
}

/*
 * Calculate size of all vars.
 */
size_t PluginManager::calc_vars_size(Plugin &plugin) {
  auto info = plugin.info_function();
  size_t size = 0;
  if (PLUGIN_TYPE_REPLACE_IMAGE != info->plugin_type) {
    return size;
  }

  for (size_t i = 0; i < info->vars_len; i++) {
    auto var = info->vars[i];
    switch (var.type) {
    case TYPE_FLOAT:
      size += sizeof(float);
      break;
    case TYPE_INT:
      size += sizeof(int32_t);
      break;
    case TYPE_BOOL:
      size += sizeof(bool);
      break;
    default:
      nhlog_fatal("Plugin's var type is invalid for = %s", info->name);
      std::abort();
    }
  }

  return size;
}
