#ifndef __OS_H__
#define __OS_H__

#include <kernel.h>
#include <stdio.h>
#include <kmt.h>

static inline void puts(const char *p) {
  for (; *p; p++) {
    _putc(*p);
  }
}

uint8_t launchd_stack[STACK_SIZE + 2 * sizeof(uint8_t)];
void launchd(void *);
thread_t launchd_thread;

#endif
