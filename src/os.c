#include <os.h>
#include <stdio.h>
#include <os/syslog.h>
#include <os/console.h>

#include <amdev.h>
#include <kmt.h>
#include <unittest.h>


static thread_t *current_thread;

static void os_init();
static void os_run();
static _RegSet *os_interrupt(_Event ev, _RegSet *regs);

MOD_DEF(os) {
  .init = os_init,
  .run = os_run,
  .interrupt = os_interrupt,
};

static void os_init() {
    int deviceit = 1;
    while (_device(deviceit) && _device(deviceit)->id != _DEV_VIDEO)
        deviceit += 1;
        
    initconsole(_device(deviceit));
    
    syslog(NULL, "NSOS/0 Version %s. Compiled with GCC %s at %s on %s.", "0.1", __VERSION__, __TIME__, __DATE__);
    syslog(NULL, "Initializing OS...");
    current_thread = NULL;
    syslog(NULL, "OS initialization is done.");
}

static void os_run() {
  
  // launchd
  //kmt->create(&launchd_thread, launchd, NULL);
    
  thread_pool[1] = &launchd_thread;
  thread_pool[1]->stack.start = (void*)launchd_stack + sizeof(uint8_t);
  thread_pool[1]->stack.end = thread_pool[1]->stack.start + STACK_SIZE;
  *(uint8_t*)(thread_pool[1]->stack.start - sizeof(uint8_t)) = STACK_MAGIC;
  *(uint8_t*)(thread_pool[1]->stack.end) = STACK_MAGIC;
  thread_pool[1]->sleep = 0;
  thread_pool[1]->current_waiting = NULL;
  thread_pool[1]->status = _make(thread_pool[1]->stack, launchd, NULL);
  syslog("OS", "launchd at 0x%08X stack starts from 0x%08X to 0x%08X", thread_pool[1], thread_pool[1]->stack.start, thread_pool[1]->stack.end);
  
  _intr_write(1); // enable interrupt
  
  
  while (1) ; // should never return
}

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
  int deviceit = 1;
  while (_device(deviceit) && _device(deviceit)->id != _DEV_INPUT)
      deviceit += 1;
  _Device *KeyboardDevice = _device(deviceit);
  _KbdReg KeyboardRegister;

  if (ev.event == _EVENT_IRQ_TIMER || ev.event == _EVENT_YIELD) {
      if (current_thread != NULL) {
          current_thread->status = regs;
      }
      current_thread = kmt->schedule();
      
      return current_thread->status;
  }
  if (ev.event == _EVENT_IRQ_IODEV) {
      // syslog("INTERRUPT", "Device I/O interrupt");
      KeyboardDevice->read(_DEVREG_INPUT_KBD, &KeyboardRegister, sizeof(KeyboardRegister));
      if (KeyboardRegister.keycode != _KEY_NONE) {
          if (KeyboardRegister.keydown == 0) {
              if (KeyboardRegister.keycode == _KEY_RETURN) {
                  key_action = 1;
              }
          }
      }
  }
  if (ev.event == _EVENT_ERROR) {
    _putc('x');
    _halt(1);
  }
  return NULL; // this is allowed by AM
}
