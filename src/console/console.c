#include <am.h>
#include <amdev.h>
#include <stdio.h>
#include <simple_lock.h>

extern unsigned char iso_font[256*16];

static spinlock_t console_lock;

static _Device *consolescreen;
static unsigned int screenwidth;
static unsigned int screenheight;
static unsigned int cursorx;
static unsigned int cursory;
static unsigned int cursorxmax;
static unsigned int cursorymax;
static char cursoron;
static char linefull;

static char screenbuf[200][200];
static unsigned char linestart;

void printstring(char *buf);
void printchar(char ch);

void clearscreen() {
    int i, j;
    _FBCtlReg ctl;
    uint32_t pixel = 0x00000000;
    
    simple_lock_try(&console_lock);
    
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
    
    simple_lock_unlock(&console_lock);
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
    int i, j;
    
    for (i = 0; i < cursorymax; i++) {
        for (j = 0; j < cursorxmax; j++) {
            cursoron = 0;
            printcharpos(screenbuf[(linestart + i) % cursorymax][j], j, i);
            cursoron = 1;
        }
    }
}

void initconsole(_Device *dev) {
    _VideoInfoReg info;
    
    simple_lock_init(&console_lock, "console_lock");
    
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
    cursoron = 1;
    linefull = 0;
    
    printcursor(0, 0);
}

void setcursor(unsigned int x, unsigned int y, char on) {
    cursorx = x;
    cursory = y;
    cursoron = on;
}

unsigned int getcursorx() {
    return cursorx;
}

unsigned int getcursory() {
    return cursory;
}

char getcursoron() {
    return cursoron;
}

void printchar(char ch) {
    int i;
    
    simple_lock_try(&console_lock);
    
    if (ch == '\n') {
        if (!linefull) {
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
        else
            linefull = 0;
    }
    else {
        printcharpos(ch, cursorx, cursory);
        screenbuf[(cursory + linestart) % cursorymax][cursorx] = ch;
        if (cursorx == cursorxmax - 1) { // new line
            linefull = 1;
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
            linefull = 0;
            cursorx += 1;
        }
    }
    if (cursoron)
        printcursor(cursorx, cursory);
        
    simple_lock_unlock(&console_lock);
}

void printstring(char *buf) {
    while (*buf) {
        printchar(*buf);
        buf++;
    }
}
