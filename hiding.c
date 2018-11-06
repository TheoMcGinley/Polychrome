#include "polychrome.h" 
#include <X11/Xmd.h>


// SHAMELESSLY STOLEN FROM AEWM
//TODO push window to stack
void hide_active_window() {
	remove_window(focused->id);
    set_wm_state(focused->id, IconicState);
}

void set_wm_state(Window win, int state) {
    CARD32 data[2];

    data[0] = state;
    data[1] = None; /* Icon? We don't need no steenking icon. */

    XChangeProperty(dpy, win, wm_state, wm_state,
        32, PropModeReplace, (unsigned char *)data, 2);
}
