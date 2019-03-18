#include "x11-window/window_driver.h"

struct windri *w;

pthread_t disp_thread_handle;

uint8_t disp_init ();

uint8_t screen_register_write (uint16_t address, uint8_t value);

void *disp_worker ();

uint8_t display_update ();

extern uint8_t tilemap[32][32];