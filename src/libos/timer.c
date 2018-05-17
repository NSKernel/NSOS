#include <am.h>
#include <amdev.h>
#include <stdint.h>
#include <stdio.h>

uint32_t getuptime32() {
    _Device *dev;
    int deviceit = 1;
    uint64_t t0 = 0;
    while (_device(deviceit) && _device(deviceit)->id != _DEV_TIMER) {
       deviceit += 1;
    }
    dev = _device(deviceit);
    _UptimeReg uptime;
    dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
    //t0 = uptime.hi;
    //t0 <<= 32;
    t0 += uptime.lo;
    return t0;
}

