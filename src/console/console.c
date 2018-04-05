#include <am.h>

extern unsigned char iso_font[256*16];

static _Device *consolescreen;

//
void setscreen(_Device *dev) {
    consolescreen = dev;
}

// Print a char to the specific position on screen
void printchar(char ch, int x, int y) {
    char selector = 0x80;
    // Pixel position is (8 + 1) * x, (16 + 1) * y
    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 8; i++) {
            _FBCtlReg ctl;
            uint32_t pixel = ((iso_font[16 * ch + j] & selector) ? 0x00FFFFFF : 0x00000000);
            ctl.x = 9 * x + i + 3;  // 3 more pixels from the left
            ctl.y = 17 * y + j + 3; // 3 more pixels from the top
            ctl.w = ctl.h = 1;
            ctl.sync = 1;
            ctl.pixels = &pixel;
            consolescreen->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
        }
        selector >>= 1;
    }
}
