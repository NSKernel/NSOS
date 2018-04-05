#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>

int printf(const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char * buf, const char *fmt, ...);

#endif
