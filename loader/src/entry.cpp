#include "inc/include.h"

Context *gctx = nullptr;

int main(int argc, char *argv[])
{
  msg("<------ DrauseHook ------>");

  gLoader = new Loader();
  gLoader->start(argc, argv);

  return 0;
}
