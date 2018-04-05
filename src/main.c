#include <am.h>
#include <amdev.h>
#include <stdio.h>
#include <os/console.h>

// TODO: implement necessary libraries
//int printf(const char *fmt, ...) {
//  for (; *fmt; fmt++) {
//    _putc(*fmt);
//  }
//  return 0;
//}

#define _LETTER(_) \
    _(A) _(B) _(C) _(D) _(E) _(F) _(G) _(H) _(I) _(J) _(K) _(L) _(M) \
    _(N) _(O) _(P) _(Q) _(R) _(S) _(T) _(U) _(V) _(W) _(X) _(Y) _(Z) 
    
#define _WRITE_LETTER(k) case _KEY_##k:\
    printchar(ASCIITable[(int)#k[0] - 'A'][(CapsLock ^ (LShiftDown | RShiftDown) ? 0 : 1)]);\
    break;

static void input_test(_Device *dev);
static void timer_test(_Device *dev);
static void video_test(_Device *dev);
static void pciconf_test(_Device *dev);
static void ata_test(_Device *dev);

char ASCIITable[26][2];
char CapsLock;
char LShiftDown;
char RShiftDown;

int main() {
  int deviceit;
  
  CapsLock = 0;
  LShiftDown = 0;
  RShiftDown = 0;

  if (_ioe_init() != 0) _halt(1);
  printf("_heap = [%08x, %08x)\n", _heap.start, _heap.end);
  for (int n = 1; ; n++) {
    _Device *dev = _device(n);
    if (!dev) break;
    printf("* Device: %s\n", dev->name);
    switch (dev->id) {
      case _DEV_INPUT: input_test(dev); break;
      case _DEV_TIMER: timer_test(dev); break;
      case _DEV_VIDEO: video_test(dev); break;
      case _DEV_PCICONF: pciconf_test(dev); break;
      case _DEV_ATA0: ata_test(dev); break;
    }
    printf("\n");
  }
  //_Device *screen;
  // init ASCII Table
  for (int i = 'A'; i < 'Z'; i++) {
      ASCIITable[i - 'A'][0] = i;
      ASCIITable[i - 'A'][1] = i + 'a' - 'A';
  }
  deviceit = 1;
  while (_device(deviceit) && _device(deviceit)->id != _DEV_VIDEO) {
      deviceit += 1;
  }
  
  initconsole(_device(deviceit));
  printstring("NS/OS 0 Console Test Space\nCopyright (C) 2018 NSKernel. All rights reserved\n\nYou may Type anything and it will show on the screen.\nBackspace is NOT AVAILABLE.\n\n");
  
  //_Device *screen;
  deviceit = 1;
  while (_device(deviceit) && _device(deviceit)->id != _DEV_INPUT) {
      deviceit += 1;
  }
  _Device *KeyboardDevice = _device(deviceit);
  _KbdReg KeyboardRegister;
  while (1) {
       KeyboardDevice->read(_DEVREG_INPUT_KBD, &KeyboardRegister, sizeof(KeyboardRegister));
       
       if (KeyboardRegister.keycode != _KEY_NONE) {
           if (KeyboardRegister.keydown == 1) {
               //printf("SHIFT = %d, CAPSLOCK = %d\n", ()
               switch (KeyboardRegister.keycode) {
                 case _KEY_GRAVE:
                   if (LShiftDown | RShiftDown)
                       printstring("~");
                   else
                       printstring("`");
                   break;
                 case _KEY_1:
                   if (LShiftDown | RShiftDown)
                       printstring("!");
                   else
                       printstring("1");
                   break;
                 case _KEY_2:
                   if (LShiftDown | RShiftDown)
                       printstring("@");
                   else
                       printstring("2");
                   break;
                 case _KEY_3:
                   if (LShiftDown | RShiftDown)
                       printstring("#");
                   else
                       printstring("3");
                   break;
                 case _KEY_4:
                   if (LShiftDown | RShiftDown)
                       printstring("$");
                   else
                       printstring("4");
                   break;
                 case _KEY_5:
                   if (LShiftDown | RShiftDown)
                       printstring("%");
                   else
                       printstring("5");
                   break;
                 case _KEY_6:
                   if (LShiftDown | RShiftDown)
                       printstring("^");
                   else
                       printstring("6");
                   break;
                 case _KEY_7:
                   if (LShiftDown | RShiftDown)
                       printstring("&");
                   else
                       printstring("7");
                   break;
                 case _KEY_8:
                   if (LShiftDown | RShiftDown)
                       printstring("*");
                   else
                       printstring("8");
                   break;
                 case _KEY_9:
                   if (LShiftDown | RShiftDown)
                       printstring("(");
                   else
                       printstring("9");
                   break;
                 case _KEY_0:
                   if (LShiftDown | RShiftDown)
                       printstring(")");
                   else
                       printstring("0");
                   break;
                 _LETTER(_WRITE_LETTER)
                 case _KEY_CAPSLOCK:
                   CapsLock ^= 1;
                   break;
                 case _KEY_RETURN:
                   printstring("\n");
                   break;
                 case _KEY_MINUS:
                   if (LShiftDown | RShiftDown)
                       printstring("_");
                   else
                       printstring("-");
                   break;
                 case _KEY_EQUALS:
                   if (LShiftDown | RShiftDown)
                       printstring("+");
                   else
                       printstring("=");
                   break;
                 case _KEY_TAB:
                   printstring("    ");
                   break;
                 case _KEY_LEFTBRACKET:
                   if (LShiftDown | RShiftDown)
                       printstring("{");
                   else
                       printstring("[");
                   break;
                 case _KEY_RIGHTBRACKET:
                   if (LShiftDown | RShiftDown)
                       printstring("}");
                   else
                       printstring("]");
                   break;
                 case _KEY_BACKSLASH:
                   if (LShiftDown | RShiftDown)
                       printstring("|");
                   else
                       printstring("\\");
                   break;
                 case _KEY_SEMICOLON:
                   if (LShiftDown | RShiftDown)
                       printstring(":");
                   else
                       printstring(";");
                   break;
                 case _KEY_APOSTROPHE:
                   if (LShiftDown | RShiftDown)
                       printstring("\"");
                   else
                       printstring("\'");
                   break;
                 case _KEY_COMMA:
                   if (LShiftDown | RShiftDown)
                       printstring("<");
                   else
                       printstring(",");
                   break;
                 case _KEY_PERIOD:
                   if (LShiftDown | RShiftDown)
                       printstring(">");
                   else
                       printstring(".");
                   break;
                 case _KEY_SLASH:
                   if (LShiftDown | RShiftDown)
                       printstring("?");
                   else
                       printstring("/");
                   break;
                 case _KEY_SPACE:
                   printstring(" ");
                   break;
                 case _KEY_LSHIFT:
                   LShiftDown = 1;
                   break;
                 case _KEY_RSHIFT:
                   RShiftDown = 1;
                   break;
                   
               }
           }
           else {
               switch (KeyboardRegister.keycode) {
                 case _KEY_LSHIFT:
                   LShiftDown = 0;
                   break;
                 case _KEY_RSHIFT:
                   RShiftDown = 0;
                   break;
               }
           }
       }
  }
  return 0;
}

static void input_test(_Device *dev) {
  printf("Input device test skipped.\n");
}

static void timer_test(_Device *dev) {
  _UptimeReg uptime;
  uint32_t t0, t1;

  dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  t0 = uptime.lo;

  for (int volatile i = 0; i < 10000000; i ++) ;

  dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  t1 = uptime.lo;

  printf("Loop 10^7 time elapse: %d ms\n", t1 - t0);
}

static void video_test(_Device *dev) {
  _VideoInfoReg info;
  dev->read(_DEVREG_VIDEO_INFO, &info, sizeof(info));
  printf("Screen size: %d x %d\n", info.width, info.height);
  for (int x = 0; x < 100; x++)
    for (int y = 0; y < 100; y++) {
      _FBCtlReg ctl;
      uint32_t pixel = 0x006a005f;
      ctl.x = info.width / 2 - 50 + x;
      ctl.y = info.height / 2 - 50 + y;
      ctl.w = ctl.h = 1;
      ctl.sync = 1;
      ctl.pixels = &pixel;
      dev->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
    }
  printf("You should see a purple square on the screen.\n");
}

static uint32_t pci_conf_read(_Device *dev, uint8_t bus, uint8_t slot,
                              uint8_t func, uint8_t offset) {
  uint32_t res;
  dev->read(_DEVREG_PCICONF(bus, slot, func, offset), &res, 4);
  return res;
}

static void pciconf_test(_Device *dev) {
  for (int bus = 0; bus < 256; bus ++)
    for (int slot = 0; slot < 32; slot ++) {
      uint32_t info = pci_conf_read(dev, bus, slot, 0, 0);
      uint16_t id = info >> 16, vendor = info & 0xffff;
      if (vendor != 0xffff) {
        printf("Get device %d:%d, id %x vendor %x", bus, slot, id, vendor);
        if (id == 0x100e && vendor == 0x8086) {
          printf(" <-- This is an Intel e1000 NIC card!");
        }
        printf("\n");
      }
    }
}

static uint8_t readb(_Device *dev, uint32_t reg) {
  uint8_t res;
  dev->read(reg, &res, 1);
  return res;
}

static uint32_t readl(_Device *dev, uint32_t reg) {
  uint32_t res;
  dev->read(reg, &res, 4);
  return res;
}

static void writeb(_Device *dev, uint32_t reg, uint8_t res) {
  dev->write(reg, &res, 1);
}

#define SECTSZ 512

static void ata_test(_Device *dev) {
  int offset = 0;
  while ((readb(dev, _DEVREG_ATA_STATUS) & 0xc0) != 0x40);
  writeb(dev, _DEVREG_ATA_NSECT,  1);
  writeb(dev, _DEVREG_ATA_SECT,   offset);
  writeb(dev, _DEVREG_ATA_CYLOW,  offset >> 8);
  writeb(dev, _DEVREG_ATA_CYHIGH, offset >> 16);
  writeb(dev, _DEVREG_ATA_DRIVE,  (offset >> 24) | 0xe0);
  writeb(dev, _DEVREG_ATA_STATUS, 0x20);
  while ((readb(dev, _DEVREG_ATA_STATUS) & 0xc0) != 0x40);
  uint32_t buf[SECTSZ / sizeof(uint32_t)];
  for (int i = 0; i < SECTSZ / sizeof(uint32_t); i++) {
    buf[i] = readl(dev, _DEVREG_ATA_DATA);
  }
  printf("Reading out the MBR:\n");
  for (int i = 0; i < SECTSZ / 16 / sizeof(uint16_t); i ++) {
    for (int j = 0; j < 16; j++) {
      printf("%04x ", ((uint16_t *)buf)[i * 16 + j] & 0xffff);
    }
    printf("\n");
  }
}
