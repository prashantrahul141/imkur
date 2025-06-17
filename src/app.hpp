#pragma once

#include "src/editor.hpp"
#include "src/ui.hpp"
#include <cstdint>

class App {
public:
  UI ui;
  Editor editor;

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

  /*
   * Retrive the global app context;
   * @returns pointer to global app context
   */
  static App *get_global_context();
};
