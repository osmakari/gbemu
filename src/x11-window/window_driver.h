#ifndef WINDOW_DRIVER_H
#define WINDOW_DRIVER_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdint.h>

#define MAX_WINDOW_COUNT 4

#define MOUSE1 1
#define MOUSE2 3
#define MOUSE3 2

struct windri {
    Display *display;
    int screen;
    Window win;
    GC gc;
    int _window_index;

    // Callbacks
    void (*onkeydown)(int key);
    void (*onmousedown)(int button);
    void (*onresize)(); // Window resize

    // Data
    int mouse_x;
    int mouse_y;

    int global_mouse_x;
    int global_mouse_y;

    uint8_t mouse_down;

    int window_width;
    int window_height;
};

unsigned long black, white;

struct windri *window_init (unsigned int width, unsigned int height, const char *title);
void window_update ();
int window_close (struct windri *w);

void draw_line (struct windri *w, int x1, int y1, int x2, int y2);
void draw_line_prop (struct windri *w, float x1, float y1, float x2, float y2);
void draw_text (struct windri *w, int x, int y, const char *text);
void draw_text_prop (struct windri *w, float x, float y, const char *text);
void draw_rectangle (struct windri *w, int x, int y, int width, int height);
void draw_rectangle_prop (struct windri *w, float x, float y, float width, float height);
void draw_arc (struct windri *w, int x, int y, int width, int height, int arc_start_angle, int arc_end_angle);
void draw_arc_prop (struct windri *w, float x, float y, float width, float height, int arc_start_angle, int arc_end_angle);
void fill_rectangle (struct windri *w, int x, int y, int width, int height);
void fill_rectangle_prop (struct windri *w, float x, float y, float width, float height);

void window_clear (struct windri *w, int x, int y, int width, int height);
void window_clear_prop (struct windri *w, float x, float y, float width, float height);

int get_mouse (struct windri *w);

#endif