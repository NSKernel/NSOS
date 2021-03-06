#include <stdarg.h>
#include <stdio.h>
#include <am.h>

#include <os/console.h>

int printf(const char *fmt, ...) {
    va_list args;
    char printbuf[5024];
    int i;
    int j = 0;
    
    va_start(args, fmt);
    i = vsprintf(printbuf, fmt, args);
    //_putc('T');
    //_putc('0' + i);
    //_putc('\n');
    for (j = 0; j < i; j++) {
        _putc(printbuf[j]);
        printchar(printbuf[j]);
    }
    va_end(args);
    return i;
}
