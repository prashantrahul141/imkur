#pragma once

#include "src/editor.hpp"
#include "src/plugins_manager.hpp"
#include "src/ui.hpp"
#include <cstdint>

class App {
public:
  UI ui;
  Editor editor;
  PluginManager plugins_manager;
  static App *global_app_context;

public:
  /*
   * Initializes all components of the application.
   */
  App();

  /*
   * Cleans up resources
   */
  ~App();

  /*
   * starts the main loop
   * @returns the exit code
   */
  const std::int32_t run();
};
