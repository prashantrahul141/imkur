#include "nhlog.h"
#include "src/config.hpp"
#include "ui.hpp"

int main(int argc, char *argv[]) {
  nhlog_init(APPLICATION_DEBUG_LEVEL, NULL);
  UI ui = UI();
  while (!ui.should_close()) {
    ui.update();
  }

  return EXIT_SUCCESS;
}
