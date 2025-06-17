#include "app.hpp"
#include "nhlog.h"
#include "src/config.hpp"

int main(int argc, char *argv[]) {
  nhlog_init(APPLICATION_DEBUG_LEVEL, NULL);
  App app = App();
  return app.run();
}
