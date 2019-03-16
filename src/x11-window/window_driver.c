#include "window_driver.h"
#include <stdio.h>
#include <stdlib.h>


struct windri *__WINDOWS[MAX_WINDOW_COUNT] = { NULL };

/*
    Section Initialize/Update/Close
*/

struct windri *window_init (unsigned int width, unsigned int height, const char *title) {
    
    struct windri *window = (struct windri*)malloc(sizeof(struct windri));
    uint8_t ok = 0;
    for(int x = 0; x < MAX_WINDOW_COUNT; x++) {
        if(__WINDOWS[MAX_WINDOW_COUNT] == NULL) {
            ok = 1;
            __WINDOWS[x] = window;
            window->_window_index = x;
            break;
        }
    }
    if(!ok) {
        printf("Failed to initialize a window: Increase MAX_WINDOW_COUNT\n");
        free(window);
        return NULL;
    }
    window->onkeydown = NULL;
    window->onmousedown = NULL;
    window->onresize = NULL;
    window->mouse_y = 0;
    window->mouse_x = 0;
    window->global_mouse_x = 0;
    window->global_mouse_y = 0;
    window->mouse_down = 0xFF;

    window->window_width = width;
    window->window_height = height;

	window->display = XOpenDisplay((char *)0);
   	window->screen = DefaultScreen(window->display);

    white = WhitePixel(window->display, window->screen);
    black = BlackPixel(window->display, window->screen);

   	window->win = XCreateSimpleWindow(window->display, DefaultRootWindow(window->display), 0, 0,	
		width, height, 5, black, white);

	XSetStandardProperties(window->display, window->win, title, "", None, NULL, 0, NULL);

	XSelectInput(window->display, window->win, ExposureMask|ButtonPressMask|KeyPressMask);

    window->gc = XCreateGC(window->display, window->win, 0,0);        

	XSetBackground(window->display, window->gc, white);
	XSetForeground(window->display, window->gc, black);

	XClearWindow(window->display, window->win);
	XMapRaised(window->display, window->win);
    XEvent ev;
    XNextEvent(window->display, &ev);
    return window;
}

void window_update () {
    for(int x = 0; x < MAX_WINDOW_COUNT; x++) {
        struct windri *w = __WINDOWS[x];
        if(w == NULL)
            continue;
        
        w->mouse_down = 0xFF;

        XEvent event;
        while(XPending(w->display)) {
            XNextEvent (w->display, &event);
            if (XFilterEvent (&event, None)) {
                continue;
            }

            switch (event.type) {
                
                case KeyPress:
                {
                    
                }
                case ButtonPress:
                {
                    if(w->onmousedown != NULL)
                        w->onmousedown(event.xbutton.button);

                    w->mouse_down = event.xbutton.button;
                    break;
                }
                case Expose:
                {

                }
                
            }
        }
        int root_x, root_y;
        int win_x, win_y;
        unsigned int mask_return;
        Window window_returned;
        
        Bool result = XQueryPointer(w->display, w->win, &window_returned,
                &window_returned, &root_x, &root_y, &win_x, &win_y,
                &mask_return);

        if(result == True) {
            w->global_mouse_x = root_x;
            w->global_mouse_y = root_y;

            w->mouse_x = win_x;
            w->mouse_y = win_y;

            
        }
    }
}

int window_close (struct windri *w) {
    if(w == NULL)
        return 0;
    
	XFreeGC(w->display, w->gc);
	XDestroyWindow(w->display, w->win);
	XCloseDisplay(w->display);
    free(w);
    __WINDOWS[w->_window_index] = NULL;
    return 1;
}

int get_mouse (struct windri *w) {
    if(w->mouse_down != 0xFF) {
        return w->mouse_down;
    }
    return 0;
}

/*
    Section Draw
    XDrawLine(dis,win,gc, x1,y1, x2,y2);
    XDrawArc(dis,win,gc,x,y, width, height, arc_start, arc_stop);
    XDrawRectangle(dis, win, gc, x, y, width, height)
    XFillArc(dis,win, gc, x, y, width, height, arc_start, arc_stop);
    XFillRectangle(dis,win,gc, x, y, width, height); 
*/

void draw_line (struct windri *w, int x1, int y1, int x2, int y2) {
    XDrawLine(w->display, w->win , w->gc, x1, y1, x2, y2);
}
// Proportional 0f-1f
void draw_line_prop (struct windri *w, float x1, float y1, float x2, float y2) {
    XDrawLine(w->display, w->win , w->gc, x1 * w->window_width, y1 * w->window_height, x2 * w->window_width, y2 * w->window_height);
}

void draw_text (struct windri *w, int x, int y, const char *text) {
    XDrawString(w->display, w->win, w->gc, x, y, text, strlen(text));
}
// Proportional 0f-1f
void draw_text_prop (struct windri *w, float x, float y, const char *text) {
    XDrawString(w->display, w->win, w->gc, x * w->window_width, y * w->window_height, text, strlen(text));
}

void draw_rectangle (struct windri *w, int x, int y, int width, int height) {
    XDrawRectangle(w->display, w->win, w->gc, x, y, width, height);
}

// Proportional 0f-1f
void draw_rectangle_prop (struct windri *w, float x, float y, float width, float height) {
    XDrawRectangle(w->display, w->win, w->gc, x * w->window_width, y * w->window_height, width * w->window_width, height * w->window_height);
}

void draw_arc (struct windri *w, int x, int y, int width, int height, int arc_start_angle, int arc_end_angle) {
    XDrawArc(w->display, w->win, w->gc, x, y, width, height, arc_start_angle, arc_end_angle);
}
// Proportional 0f-1f
void draw_arc_prop (struct windri *w, float x, float y, float width, float height, int arc_start_angle, int arc_end_angle) {
    XDrawArc(w->display, w->win, w->gc, x * w->window_width, y * w->window_height, width * w->window_width, height * w->window_height, arc_start_angle, arc_end_angle);
}

void fill_rectangle (struct windri *w, int x, int y, int width, int height) {
    XFillRectangle(w->display, w->win, w->gc, x, y, width, height);
} 

// Proportional 0f-1f
void fill_rectangle_prop (struct windri *w, float x, float y, float width, float height) {
    XFillRectangle(w->display, w->win, w->gc, x * w->window_width, y * w->window_height, width * w->window_width, height * w->window_height);
}

void window_clear (struct windri *w, int x, int y, int width, int height) {
     XClearArea(w->display, w->win, x, y, width, height, False);
}

void window_clear_prop (struct windri *w, float x, float y, float width, float height) {
     XClearArea(w->display, w->win, x * w->window_width, y * w->window_height, width * w->window_width, height * w->window_height, False);
}