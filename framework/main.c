#include <kernel.h>
#include <unittest.h>

int main() {
  // AM initialization
  _ioe_init();
  if (os->interrupt) _asye_init(os->interrupt); 

  // OS module initialization
  if (os->init) os->init();
  if (pmm->init) pmm->init();
  if (kmt->init) kmt->init();
  if (vfs->init) vfs->init();

  //kmt_test();

  // call os->run()
  if (os->run) os->run();

  _halt(1); // should not reach here
  return -1;
}
