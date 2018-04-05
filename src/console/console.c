#include <am.h>
#include <amdev.h>
#include <stdio.h>

extern unsigned char iso_font[256*16];

static _Device *consolescreen;
static unsigned int screenwidth;
static unsigned int screenheight;
static unsigned int cursorx;
static unsigned int cursory;
static unsigned int cursorxmax;
static unsigned int cursorymax;

static char screenbuf[200][200];
static unsigned char linestart;

void printstring(char *buf);

void clearscreen() {
    int i, j;
    _FBCtlReg ctl;
    uint32_t pixel = 0x00000000;
    
    for (i = 0; i < cursorymax; i++) {
        for (j = 0; j < cursorxmax; j++) {
            screenbuf[i][j] = ' ';
        }
        screenbuf[i][cursorxmax] = '\0';
    }
    
    ctl.x = 0;
    ctl.y = 0;
    ctl.w = 1;
    ctl.h = 1;
    ctl.sync = 1;
    ctl.pixels = &pixel;
    for (i = 0; i < screenwidth; i++) {
        for (j = 0; j < screenheight; j++) {
            ctl.x = i;
            ctl.y = j;
            consolescreen->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
        }
    }
}

// Print a char to the specific position on screen
void printcharpos(char ch, int x, int y) {
    unsigned char selector = 1;
    int i, j;
    // Pixel position is 8 * x, 16 * y(additional space between lines)
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

void printcursor(int x, int y) {
    int i, j;
    // Pixel position is 8 * x, 16 * y(additional space between lines)
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 8; i++) {
            _FBCtlReg ctl;
            uint32_t pixel = 0x00FFFFFF;
            ctl.x = 8 * x + i + 3;  // 3 more pixels from the left
            ctl.y = 16 * y + j + 3; // 3 more pixels from the top
            ctl.w = ctl.h = 1;
            ctl.sync = 1;
            ctl.pixels = &pixel;
            consolescreen->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
        }
    }
}

void refreshscreen() {
    int i;
    
    cursorx = 0;
    cursory = 0;
    for (i = 0; i < cursorymax - 1; i++) {
        printstring(screenbuf[(linestart + i) % cursorymax]);
    }
    for (i = 0; i < cursorxmax; i++) {
        printcharpos(' ', i, cursorymax - 1);
    }
}

//
void initconsole(_Device *dev) {
    _VideoInfoReg info;
    
    dev->read(_DEVREG_VIDEO_INFO, &info, sizeof(info));
    screenwidth = info.width;
    screenheight = info.height;
    
    consolescreen = dev;
    cursorx = 0;
    cursory = 0;
    cursorxmax = (info.width - 6) / 8;
    cursorymax = (info.height - 6) / 16;
    linestart = 0;
    clearscreen();
    refreshscreen();
    
    cursorx = 0;
    cursory = 0;
    
    printf("Screen device id is %08X\n", dev->id);
}

void printchar(char ch) {
    int i;
    
    if (ch == '\n') {
        printcharpos(ch, cursorx, cursory);
        cursorx = 0;
        if (cursory == cursorymax - 1) { 
            for (i = 0; i < cursorxmax; i++) {
                screenbuf[(linestart + cursory + 1) % cursorymax][i] = ' ';
            }
            linestart = (linestart + 1) % cursorymax;
            refreshscreen();
        }
        else {
            cursory += 1;
        }
    }
    else {
        printcharpos(ch, cursorx, cursory);
        screenbuf[(cursory + linestart) % cursorymax][cursorx] = ch;
        if (cursorx == cursorxmax - 1) { // new line
            cursorx = 0;
            if (cursory == cursorymax - 1) {
                for (i = 0; i < cursorxmax; i++) {
                    screenbuf[(linestart + cursory + 1) % cursorymax][i] = ' ';
                }
                linestart = (linestart + 1) % cursorymax;
                refreshscreen();
            }
            else {
                cursory += 1;
            }
        }
        else {
            cursorx += 1;
        }
    }
    printcursor(cursorx, cursory);
}

void printstring(char *buf) {
    while (*buf) {
        printchar(*buf);
        buf++;
    }
}
