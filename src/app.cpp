#include "app.hpp"
#include "src/editor.hpp"
#include "src/plugins_manager.hpp"
#include <cstdlib>

static App *global_app_context = nullptr;

/*
 * Initializes all components of the application.
 */
App::App() : ui(UI()), editor(Editor()), plugins_manager(PluginManager()) {
  global_app_context = this;
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

/*
 * Retrive the global app context;
 * @returns pointer to global app context
 */
App *App::get_global_context() { return global_app_context; }
