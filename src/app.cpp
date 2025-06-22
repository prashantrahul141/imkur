#include "app.hpp"
#include "src/editor.hpp"
#include "src/plugins_manager.hpp"
#include <cstdlib>

App *App::global_app_context = nullptr;

/*
 * Initializes all components of the application.
 */
App::App() : ui(UI()), editor(Editor()), plugins_manager(PluginManager()) {
  if (nullptr == global_app_context) {
    global_app_context = this;
  }
}

/*
 * Cleans up resources
 */
App::~App() {}

/*
 * starts the main loop
 * @returns the exit code
 */
const std::int32_t App::run() {
  while (!ui.should_close()) {
    ui.update();
  }
  return EXIT_SUCCESS;
}
