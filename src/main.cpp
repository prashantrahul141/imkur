#include "app.hpp"
#include "nhlog.h"
#include "src/config.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG_BUILD
  void *fd = NULL;
#else
  void *fd = fopen("logs.txt", "w");
#endif
  nhlog_init(APPLICATION_DEBUG_LEVEL, (FILE *)fd);
  App app = App();
  return app.run();
}
