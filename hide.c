// all hiding/reshowing functions can be found below
#include "polychrome.h" 
#include <X11/Xmd.h>

// SHAMELESSLY STOLEN FROM AEWM
//TODO push window to stack
//TODO add these functions to polychrome
void hideFocusedWindow() {
	hide(focused->id);
}

void hide(Window win) {
	removeWindow(win)
    setWindowState(win, IconicState);

}

void showNextHidden() {
	//TODO pop stack to show win
	//setWindowState(stack.pop()->id, NormalState)
}

void setWindowState(Window win, int state) {
    CARD32 data[2];

    data[0] = state;
    data[1] = None; /* Icon? We don't need no steenking icon. */

    XChangeProperty(dpy, win, wm_state, wm_state,
        32, PropModeReplace, (unsigned char *)data, 2);
}
