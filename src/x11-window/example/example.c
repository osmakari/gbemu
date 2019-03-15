#include "../window_driver.h"
#include <stdio.h>
#include <time.h>

struct windri *w;

void onresize () {
    printf("Window resized %i %i\n", w->window_width, w->window_height);
}

int main () {

    w = window_init(200, 200, "This is a window");
    draw_line_prop(w, 0.1, 0.1, 0.9, 0.9);
    draw_text_prop(w, 0.1, 0.8, "text");
    w->onresize = onresize;
    int lx = 0, ly = 0;
    while(1) {
        window_update ();
        if(get_mouse(w)) {
            window_clear(w, lx-5, ly-5, 11, 11);
            draw_rectangle(w, w->mouse_x - 5, w->mouse_y - 5, 10, 10);
            lx = w->mouse_x;
            ly = w->mouse_y;
        }
    }
    return 0;
}