#include <am.h>
#include <amdev.h>
#include <stdio.h>

extern unsigned char iso_font[256*16];

static _Device *consolescreen;
static unsigned int cursorx;
static unsigned int cursory;
static unsigned int cursorxmax;
static unsigned int cursorymax;

//
void initconsole(_Device *dev) {
    _VideoInfoReg info;
    dev->read(_DEVREG_VIDEO_INFO, &info, sizeof(info));
    printf("Screen size: %d x %d\n", info.width, info.height);
    
    consolescreen = dev;
    cursorx = 0;
    cursory = 0;
    cursorxmax = (info.width - 6) / 8;
    cursorymax = (info.height - 6) / 17;
    
    printf("Screen device id is %08X\n", dev->id);
}

// Print a char to the specific position on screen
void printchar(char ch, int x, int y) {
    unsigned char selector = 1;
    int i, j;
    // Pixel position is 8 * x, 17 * y(additional space between lines)
    for (j = 0; j < 16; j++) {
        selector = 1;
        for (i = 0; i < 8; i++) {
            _FBCtlReg ctl;
            uint32_t pixel = ((iso_font[16 * ch + j] & selector) ? 0x00FFFFFF : 0x00000000);
            ctl.x = 8 * x + i + 3;  // 3 more pixels from the left
            ctl.y = 16 * y + j + 3; // 3 more pixels from the top
            ctl.w = ctl.h = 1;
            ctl.sync = 1;
            ctl.pixels = &pixel;
            consolescreen->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
            selector <<= 1;
        }
    }
}

void printstring(char *buf) {
    while (*buf) {
        if (*buf == '\n') {
            cursorx = 0;
            if (cursory == cursorymax) {
                cursory = 0;
            }
            else {
                cursory += 1;
            }
        }
        else {
            printchar(*buf, cursorx, cursory);
            if (cursorx == cursorxmax) { // new line
                cursorx = 0;
                if (cursory == cursorymax) {
                    cursory = 0;
                }
                else {
                    cursory += 1;
                }
            }
            else {
                cursorx += 1;
            }
        }
    }
    
}
