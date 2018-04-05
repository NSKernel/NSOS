#include <stdarg.h>
#include <stdio.h>

static char printbuf[1024];

int printf(const char *fmt, ...) {
    va_list args;
    int i;
    int j = 0;
    
    va_start(args, fmt);
    i = vsprintf(printbuf, fmt, args);
    for (j = 0; j < i; j++)
        _putc(printbuf[i]);
    va_end(args);
}
