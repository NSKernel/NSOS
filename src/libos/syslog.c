#include <stdarg.h>
#include <stdio.h>
#include <os.h>
#include <os/timer.h>


void syslog(const char *who, const char *fmt, ...) {
    char ringbuf[1024];
    va_list args;
    va_start(args, fmt); 
    ringbuf[vsprintf(ringbuf, fmt, args)] = 0;
    
    if (who != NULL)
        printf("[%5d.%03d] %s: %s\n", getuptime32() / 1000, getuptime32() % 1000, who, ringbuf);
    else 
        printf("[%5d.%03d] %s\n", getuptime32() / 1000, getuptime32() % 1000, ringbuf);
    va_end(args);
}
