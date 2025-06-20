#include "src/plugins_manager.hpp"
#include "nhlog.h"
#include "src/config.hpp"
#include <cstdlib>
#include <filesystem>

/*
 * Constructor
 */
PluginManager::PluginManager() {
  if (!std::filesystem::exists(PATH_PLUGINS_DIR) ||
      !std::filesystem::is_directory(PATH_PLUGINS_DIR)) {
    nhlog_fatal("No plugins directory found.");
    std::abort();
  }
  const auto plugins = std::filesystem::directory_iterator(PATH_PLUGINS_DIR);
  for (const auto &plugin_path : plugins) {
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
      nhlog_error("PluginManager: plugin handler is null");
      continue;
    }

    if (!PluginManager::is_plugin_valid(p)) {
      nhlog_error("PluginManager: invalid plugin = %s",
                  plugin_path.path().c_str());
      DL_CLOSE(abs_path.c_str());
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
    break;
  }
  default: {
    nhlog_error("PluginManager: invalid plugin type");
    return false;
  }
  }

  return true;
}
